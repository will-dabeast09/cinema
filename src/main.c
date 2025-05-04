typedef struct global global_t;
#define usb_callback_data_t global_t

#include <fileioc.h>
#include <graphx.h>
#include <msddrvce.h>
#include <tice.h>
#include <usbdrvce.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PARTITIONS 32
#define BLOCK_SIZE 512
#define APPVAR "SSCINEMA"

//Initialize the sprite buffers
gfx_sprite_t *sprite_buffer_1;
gfx_sprite_t *sprite_buffer_2;


struct global
{
    usb_device_t usb;
    msd_t msd;
};
enum { USB_RETRY_INIT = USB_USER_ERROR };
static void putstr(char *str)
{
    os_PutStrFull(str);
    os_NewLine();
}
static usb_error_t handleUsbEvent(usb_event_t event, void *event_data,
                                  usb_callback_data_t *global)
{
    switch (event)
    {
        case USB_DEVICE_DISCONNECTED_EVENT:
            putstr("usb device disconnected");
            if (global->usb)
                msd_Close(&global->msd);
            global->usb = NULL;
            break;
        case USB_DEVICE_CONNECTED_EVENT:
            putstr("usb device connected");
            return usb_ResetDevice(event_data);
        case USB_DEVICE_ENABLED_EVENT:
            global->usb = event_data;
            putstr("usb device enabled");
            break;
        case USB_DEVICE_DISABLED_EVENT:
            putstr("usb device disabled");
            return USB_RETRY_INIT;
        default:
            break;
    }

    return USB_SUCCESS;
}


//declare functions
void palette_callback(msd_error_t error, struct msd_transfer *xfer);
void image_callback(msd_error_t error, struct msd_transfer *xfer);
void cinema_Save(uint32_t *lba, uint8_t var);
uint32_t cinema_Load(uint8_t var);

int main(void)
{
    //declare variables   
    static char buffer[212];
    static global_t global;
    uint16_t palette_buffer_1[256] = {0};
    uint16_t palette_buffer_2[256];
    bool render = false;
    msd_transfer_t xfer_palette;
    msd_transfer_t xfer_image;
    msd_info_t msdinfo;
    usb_error_t usberr;
    msd_error_t msderr;

    xfer_palette.msd = &global.msd;
    xfer_palette.lba = 0;
    xfer_palette.count = 1;
    xfer_palette.callback = palette_callback;

    xfer_image.msd = &global.msd;
    xfer_image.lba = 1;
    xfer_image.count = 30;
    xfer_image.callback = image_callback;
    xfer_image.userptr = &render;

   //usb & msd initialization
    memset(&global, 0, sizeof(global_t));
    os_SetCursorPos(1, 0);

    // usb initialization loop; waits for something to be plugged in
    do
    {
        global.usb = NULL;

        usberr = usb_Init(handleUsbEvent, &global, NULL, USB_DEFAULT_INIT_FLAGS);
        if (usberr != USB_SUCCESS)
        {
            putstr("usb init error.");
            goto usb_error;
        }

        while (usberr == USB_SUCCESS)
        {
            if (global.usb != NULL)
                break;

            // break out if a key is pressed
            if (os_GetCSC())
            {
                putstr("exiting cinema, press a key");
                goto usb_error;
            }

            usberr = usb_WaitForInterrupt();
        }
    } while (usberr == USB_RETRY_INIT);

    if (usberr != USB_SUCCESS)
    {
        putstr("usb enable error.");
        goto usb_error;
    }

    // initialize the msd device
    msderr = msd_Open(&global.msd, global.usb);
    if (msderr != MSD_SUCCESS)
    {
        putstr("failed opening msd");
        goto usb_error;
    }

    putstr("opened msd");

    // get block count and size
    msderr = msd_Info(&global.msd, &msdinfo);
    if (msderr != MSD_SUCCESS)
    {
        putstr("error getting msd info");
        goto msd_error;
    }

    // print msd sector number and size
    sprintf(buffer, "block size: %u bytes", (uint24_t)msdinfo.bsize);
    putstr(buffer);
    sprintf(buffer, "num blocks: %u", (uint24_t)msdinfo.bnum);
    putstr(buffer);
    
    os_ClrHome();

    {   //save menu
        uint8_t var = ti_Open(APPVAR, "r+");
        if (var) {
            putstr("Resume where you left off?");
            putstr("Other - YES        0 - NO");

            while (1) {
                uint8_t key = os_GetCSC();
                if (key) {
                    if (key == sk_0) {
                        break;
                    }
                    else {
                        xfer_palette.lba = cinema_Load(var);
                        xfer_image.lba = xfer_palette.lba + 1;
                        ti_SetGCBehavior(NULL, NULL);
                        ti_SetArchiveStatus(1, var);
                        break;
                    }
                }
            }
            ti_Close(var);
        }
    }  
    {   //graphx
        gfx_Begin();
        gfx_SwapDraw();
        gfx_SetDrawBuffer();
        gfx_ZeroScreen();
        gfx_SwapDraw();
        gfx_SetDrawBuffer();
        gfx_ZeroScreen();

        //Allocate memory for the sprites
        sprite_buffer_1 = gfx_MallocSprite(160, 96);
        sprite_buffer_2 = gfx_MallocSprite(160, 96);

        //Load the first frame
        xfer_palette.buffer = palette_buffer_1;
        xfer_image.buffer = &sprite_buffer_1->data;
        
        msderr = msd_ReadAsync(&xfer_palette);
        if (msderr != MSD_SUCCESS) {
            putstr("error queueing msd (palette)");
            goto msd_error;
        }
        msderr = msd_ReadAsync(&xfer_image);
        if (msderr != MSD_SUCCESS) {
            putstr("error queueing msd (image)");
            goto msd_error;
        }

        while(!render){
            usb_HandleEvents();
        }

        render = false;

        //Main Loop
        while (!os_GetCSC()) {
            //set the buffers
            gfx_SetDrawBuffer();
            xfer_image.buffer = &sprite_buffer_2->data;
            xfer_palette.buffer = palette_buffer_2;

            //queue the async reads
            msderr = msd_ReadAsync(&xfer_palette);
            if (msderr != MSD_SUCCESS) {
                putstr("error queueing msd (palette)");
                goto msd_error;
            }
            msderr = msd_ReadAsync(&xfer_image);
            if (msderr != MSD_SUCCESS) {
                putstr("error queueing msd (image)");
                goto msd_error;
            }
            
            //display the image while async is reading
            gfx_ScaledSprite_NoClip(sprite_buffer_1, 0, 24, 2, 2);
            gfx_SwapDraw();
            gfx_Wait();
            gfx_SetPalette(palette_buffer_1, 512, 0);

            //run the callbacks
            while(!render){
                usb_HandleEvents();
            }
            render = false;

            //set to read to the other buffers (double buffering)
            gfx_SetDrawBuffer();
            xfer_image.buffer = &sprite_buffer_1->data;
            xfer_palette.buffer = palette_buffer_1;

            
            msderr = msd_ReadAsync(&xfer_palette);
            if (msderr != MSD_SUCCESS) {
                putstr("error queueing msd (palette)");
                goto msd_error;
            }
            msderr = msd_ReadAsync(&xfer_image);
            if (msderr != MSD_SUCCESS) {
                putstr("error queueing msd (image)");
                goto msd_error;
            }
            
            //display the image while async is reading
            gfx_ScaledSprite_NoClip(sprite_buffer_2, 0, 24, 2, 2);
            gfx_SwapDraw();
            gfx_Wait();
            gfx_SetPalette(palette_buffer_2, 512, 0);

            //run the callbacks
            while(!render){
                usb_HandleEvents();
            }
            render = false;

        }
        gfx_End();
    }
    {   //save place
        uint8_t var = ti_Open(APPVAR, "w");
        if (var) {
            //set the var to ram so ti_open can write to it
            ti_SetGCBehavior(NULL, NULL);
            ti_SetArchiveStatus(0, var);
            cinema_Save(&xfer_palette.lba, var);
            //re-archive the variable
            ti_SetArchiveStatus(1, var);
            ti_Close(var);
        }
    }

    msd_Close(&global.msd);
    usb_Cleanup();
    return 0;
    

msd_error:
    // close the msd device
    msd_Close(&global.msd);
    usb_Cleanup();

    while (!os_GetCSC());
    
    return 0;

usb_error:
    // cleanup usb
    usb_Cleanup();

    while (!os_GetCSC());

    return 0;
}

void palette_callback(msd_error_t error, struct msd_transfer *xfer) {
    xfer->lba += 31;
    if (error != MSD_SUCCESS) {
        gfx_End();
        switch (error) {
            case MSD_ERROR_INVALID_PARAM:
                putstr("invalid param (palette)");
                break;
            case MSD_ERROR_USB_FAILED:
                putstr("usb failed (palette)");
                break;
            case MSD_ERROR_SCSI_FAILED:
                putstr("scsi failed (palette)");
                break;
            case MSD_ERROR_SCSI_CHECK_CONDITION:
                putstr("scsi check condition (palette)");
                break;
            case MSD_ERROR_NOT_SUPPORTED:
                putstr("not supported (palette)");
                break;
            case MSD_ERROR_INVALID_DEVICE:
                putstr("invalid device (palette)");
                break;
            case MSD_ERROR_TIMEOUT:
                putstr("timeout (palette)");
                break;
            default:
                putstr("Unknown error.");
                break;
        }
    }
}
void image_callback(msd_error_t error, struct msd_transfer *xfer) {
    xfer->lba += 31;
    *(bool*)xfer->userptr = true;
    if (error != MSD_SUCCESS) {
        gfx_End();
        switch (error) {
            case MSD_ERROR_INVALID_PARAM:
                putstr("invalid param (image)");
                break;
            case MSD_ERROR_USB_FAILED:
                putstr("usb failed (image)");
                break;
            case MSD_ERROR_SCSI_FAILED:
                putstr("scsi failed (image)");
                break;
            case MSD_ERROR_SCSI_CHECK_CONDITION:
                putstr("scsi check condition (image)");
                break;
            case MSD_ERROR_NOT_SUPPORTED:
                putstr("not supported (image)");
                break;
            case MSD_ERROR_INVALID_DEVICE:
                putstr("invalid device (image)");
                break;
            case MSD_ERROR_TIMEOUT:
                putstr("timeout (image)");
                break;
            default:
                putstr("Unknown error.");
                break;
        }
    }
}
void cinema_Save(uint32_t *lba, uint8_t var) {
    ti_Write(lba, sizeof(uint32_t), 1, var);
}
uint32_t cinema_Load(uint8_t var) {
    uint32_t lba;

    ti_Read(&lba, sizeof(uint32_t), 1, var);

    return lba;
}

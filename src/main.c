typedef struct global global_t;
#define usb_callback_data_t global_t

#include <usbdrvce.h>
#include <msddrvce.h>
#include <tice.h>
#include <graphx.h>

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


#define MAX_PARTITIONS 32
#define BLOCK_SIZE 512

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

void palette_callback(msd_error_t error, struct msd_transfer *xfer) {
    if (error == MSD_SUCCESS) {
        xfer->lba += 151;
        *(bool*)xfer->userptr = true;
    } else {
        putstr("error reading palette");
    }
}

void image_callback(msd_error_t error, struct msd_transfer *xfer) {
    if (error == MSD_SUCCESS) {
        xfer->lba += 151;
        *(bool*)xfer->userptr = true;
    } else {
        putstr("error reading image");
    }
}

int main(void)
{
    //variable definitions
        static char buffer[212];
        static global_t global;
        msd_transfer_t xfer_palette;
        msd_transfer_t xfer_image;
        uint16_t palette[256];
        bool copy_palette = false;
        bool render = false;
        msd_info_t msdinfo;
        usb_error_t usberr;
        msd_error_t msderr;

        xfer_palette.msd = &global.msd;
        xfer_palette.lba = 0;
        xfer_palette.count = 1;
        xfer_palette.callback = palette_callback;
        xfer_palette.userptr = &copy_palette;

        xfer_image.msd = &global.msd;
        xfer_image.lba = 1;
        xfer_image.count = 150;
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
                    putstr("exiting demo, press a key");
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


    //graphx
    {
        gfx_Begin();
        gfx_SwapDraw();

        xfer_palette.buffer = palette;
        
        {
            xfer_image.buffer = gfx_vbuffer;
            
            msd_ReadAsync(&xfer_palette);
            if (msderr != MSD_SUCCESS) {
                putstr("error queueing msd (palette)");
                goto msd_error;
            }
            msd_ReadAsync(&xfer_image);
            if (msderr != MSD_SUCCESS) {
                putstr("error queueing msd (image)");
                goto msd_error;
            }

            while (!copy_palette) {
                usb_HandleEvents();
            }

            gfx_SetPalette(palette, 512, 0);

            while (!render) {
                usb_HandleEvents();
            }

            gfx_SwapDraw();
            render = false;
            copy_palette = false;

            xfer_image.buffer = gfx_vbuffer;
            
            msd_ReadAsync(&xfer_palette);
            msd_ReadAsync(&xfer_image);

            while (!copy_palette || !render) {
                usb_HandleEvents();
            }

        }

        while (!os_GetCSC())
        {
            gfx_SetPalette(palette, 512, 0);
            gfx_SwapDraw();
            render = false;
            copy_palette = false;

            xfer_image.buffer = gfx_vbuffer;
            
            msd_ReadAsync(&xfer_palette);
            msd_ReadAsync(&xfer_image);

            while (!copy_palette || !render) {
                usb_HandleEvents();
            }

        }
            
        
        gfx_End();
    }


    msd_Close(&global.msd);
    usb_Cleanup();
    return 0;
    

msd_error:
    // close the msd device
    msd_Close(&global.msd);

usb_error:
    // cleanup usb
    usb_Cleanup();

    while (!os_GetCSC());

    return 0;
}

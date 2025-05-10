# Cinema

USB Video Player for the TI-84 Plus CE  

![Demo 1](media/dreamworks.gif) ![Demo 2](media/test_drive.gif)

## Overview

Cinema is a video player application that allows you to watch videos on your TI-84 Plus CE calculator using a USB thumb drive.

## Installation Instructions

1. **Prepare your USB drive**
   - Unformat and wipe your thumb drive completely

2. **Convert your video**
   - Use FFmpeg to convert your video into individual frames
   - Aim for approximately 10-11 FPS for optimal playback

3. **Process the frames**
   - Run [FBin](https://github.com/will-dabeast09/fbin) to convert frames to the required format
   - This will output a binary file ready for the calculator

4. **Write to thumb drive**
   - Use [HDD Raw Copy Tool](https://hddguru.com/software/HDD-Raw-Copy-Tool/) to transfer the binary file to your thumb drive

5. **Install on calculator**
   - Transfer CINEMA.8xp to your TI-84 Plus CE calculator

## Usage

1. Run CINEMA on your calculator
2. Insert your prepared USB drive
3. If you have run it before, press 0 to restart the video, and any other key to resume.
3. Video will begin playing
4. Press any key (except ON) to exit

## Technical Specifications

- **Version:** Current
- **Framerate:** 10-11 fps at 160 x 96 quality
- **Color depth:** 256 colors per image

## Links

- [FBin GitHub Repository](https://github.com/will-dabeast09/fbin)
- [HDD Raw Copy Tool](https://hddguru.com/software/HDD-Raw-Copy-Tool/)

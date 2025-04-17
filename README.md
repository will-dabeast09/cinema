# Cinema

USB Video Player for the TI-84 Plus CE

## Overview

Cinema is a video player application that allows you to watch videos on your TI-84 Plus CE calculator using a USB thumb drive. It displays pre-processed video frames at approximately 3.5 FPS with 256 colors per image.

## ⚠️ Important Warning

**DO NOT unplug the USB drive while Cinema is running!** This will cause your calculator to freeze, requiring a RAM reset to restore functionality.

## Installation Instructions

1. **Prepare your USB drive**
   - Unformat and wipe your thumb drive completely

2. **Convert your video**
   - Use FFmpeg to convert your video into individual frames
   - Aim for approximately 3.5 FPS for optimal playback

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
3. Video will begin playing automatically
4. Press any key (except ON) to exit
5. **Always exit properly before removing the USB drive** to avoid calculator freeze

## Technical Specifications

- **Version:** Current
- **Framerate:** 3.5 FPS
- **Color depth:** 256 colors per image
- **Memory usage:** Approximately 6 KB

## Links

- [FBin GitHub Repository](https://github.com/will-dabeast09/fbin)
- [HDD Raw Copy Tool](https://hddguru.com/software/HDD-Raw-Copy-Tool/)

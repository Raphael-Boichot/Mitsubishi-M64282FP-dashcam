# These files have to be copied at the root of SD card.

## Powershell scripts made by [HerrZatacke](https://github.com/HerrZatacke)

- **Upscaler_Camera.ps1** converts 1x BMP files from /Camera folder to 8x PNG;
- **Upscaler_MS.ps1** converts 1x BMP files from /MS (Motion Sensor) folder to 8x PNG; 
- **Upscaler_TL.ps1** converts 1x BMP files from /TL (Time Lapse) folder to 8x PNG; 

## json configuration file

**config.json** is the general configuration file for the DashBoy Camera. User can keep the current values without modifying anything. Advanced user are advised to look into code what is the effect of each. **Value can be changed but not the length of tables. Values MUST be entered as decimal.** Here is the signification of each variable in detail:

- **timelapserawrecordingMode:** 1 = recording to .raw file, 0 = recording to 1x bmp files with a limit of 1024 files per folder
- **timelapseDelay:** list of actions associated with the timelapse button. A value up to zero is a delay for timelapse in ms, -1 is the regular camera mode and -2 is the motion sensor mode;
- **prettyborderMode:** 1 = pretty borders, 0 = image without border;
- **nightMode:** 1 = allows to go decrease the sensor clock to over expose image at night, 0 = constant sensor clock;
- **motiondetectionThreshold:** value for the motion detection threshold. 0 = detect everything, 1 = detect nothing. 0.025 is a good starting value, the lower, the lore sensitive to motion;
- **2dEnhancement:** 1 = 2D enhancement activated, 0 = deactivated - Valid only in single register strategy;
- **hdrExposures:** list of 8 exposures in HDR mode, in multiplier of the last best knowbn exposure. It is recommended to go from -1EV to +1EV;
- **ditherMatrix:** dithering matrix used in single register strategy, derived from a 4x4 bayer matrix;
- **gameboycameraMode:** 0 = single register strategy, 1 = Game Boy Camera strategy with variable registers;
- **lowvoltageThreshold:** voltage threshold for black level in non dithered Game Boy Camera strategy (0=0.0 Volts, 255 = 3.3 volts);
- **highvoltageThreshold:** voltage threshold for white level in non dithered Game Boy Camera strategy (0=0.0 Volts, 255 = 3.3 volts);
- **lowditherMatrix:** dithering matrix used in Game Boy Camera strategy - low ligth or high exposure times, derived from a 4x4 bayer matrix; 
- **highditherMatrix:** dithering matrix used in Game Boy Camera strategy - high ligth or low exposure times, derived from a 4x4 bayer matrix; 
- **fixedExposure:** 0 = auto-exposure, 1 = fixed exposure, for astrophotography for example;
- **fixedDelay:** max is 65535 (about 1 second), min is 48 (about 0.768 ms);
- **fixedDivider:** clock divider, the more, the longer the exposure. For example 2 double the exposure time;

## Convert images to movie, script made by [HerrZatacke](https://github.com/HerrZatacke)

Just install ffmpeg from Powershell ran as administrator following this method: **[Method 2: Install FFmpeg via Chocolatey](https://adamtheautomator.com/install-ffmpeg/)**.
Then run the following command into you image folder (png or bmp, adapt it):
    
    ffmpeg -i %07d.png -s 1024x896 -sws_flags neighbor -r 30 -vcodec libx264 -c:v libx264 -crf 30 -pix_fmt yuv420p output.mp4
    
Or use the [Matlab Movie Maker](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/Matlab%20only/Movie_Maker_Matlab_only.m) (NOT GNU Octave compatible).

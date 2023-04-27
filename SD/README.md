# Some more explanations

These files have to be copied at the root of SD card.

- **convert.ps1** and **covertraw.ps1** are Powershell scripts that allows to convert bmp and raw files to png and upscale them from the /Camera and /Raw_data folders. To run them, simply right click on them and select "Run as Powershell". 

- **config.json** is the general configuration file for the DashBoy Camera. Here is the signification of each variable in detail:

- **timelapserawrecordingMode:** 1 = recording to .raw file, 0 = recording to 1x bmp files with a limit of 1024 files per folder
- **timelapseDelay:** the list of actions associated with the timelapse button. A value up to zero is a delay for timelapse in ms, -1 is the regular camera mode and -2 is the motion sensor mode;
- **prettyborderMode:** 1 = pretty borders, 0 = image without border;
- **nightMode:** 1 = allows to go decrease the sensor clock to over expose image at night, 0 = constant sensor clock;
- **motiondetectionThreshold:** value for the motion detection threshold. 0 = detect everything, 1 = detect nothing. 0.025 is a good starting value, the lower, the lore sensitive to motion;
"2dEnhancement":1,
"hdrExposures":[0.5, 0.69, 0.79, 1, 1, 1.26, 1.44, 2],
"ditherMatrix":[ 42, 94, 155, 81, 139, 202, 51, 105, 166, 90, 151, 214, 68, 124, 186, 55, 109, 170, 77, 135, 198, 64, 120, 182, 48, 101, 162, 87, 147, 210, 45, 97, 158, 84, 143, 206, 74, 132, 194, 61, 116, 178, 71, 128, 190, 58, 113, 174 ],
"gameboycameraMode":1,
"lowvoltageThreshold":125,
"highvoltageThreshold":195,
"lowditherMatrix": [140,152,172,149,167,219,142,155,183,151,170,231,146,162,203,143,157,187,148,165,215,145,160,199,141,154,179,150,169,227,140,153,175,149,168,223,147,164,211,144,159,195,146,163,207,143,158,191],
"highditherMatrix":[137,146,162,143,158,198,138,149,171,145,161,207,141,154,186,139,150,174,143,157,195,140,153,183,138,148,168,144,160,204,137,147,165,144,159,201,142,156,192,140,152,180,142,155,189,139,151,177],
"fixedExposure":0,
"fixedDelay":65000,
"fixedDivider":4


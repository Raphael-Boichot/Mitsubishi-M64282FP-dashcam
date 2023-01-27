# Mitsubishi-M64282FP-dashcam

A mobile recorder for the Mitsubishi M64282FP sensor of the Game Boy Camera, For the Raspberry Pi Pico. The code was developped under the Arduino IDE with the Earle F. Philhower Raspberry Pi Pico Arduino core. TFT Display driven with the Bodmer TFT_eSPI library.

# Multi-systems installation guide

- Install the last [Arduino IDE](https://www.arduino.cc/en/software)
- Install the [Earle F. Philhower Raspberry Pi Pico Arduino core for Arduino IDE](Earle F. Philhower Raspberry Pi Pico Arduino core) via the Arduino Board manager (see [installation guide](https://github.com/earlephilhower/arduino-pico#installing-via-arduino-boards-manager)).
- Import or install the [Bodmer TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI).
- Locate the TFT_eSPI library :
    \Arduino\libraries\TFT_eSPI folder in your documents
- copy the [Mitsubishi_M64282FP_dashcam_TFT_eSPI_setup.h]() configuration file for the TFT display in this folder.
- edit the User_Setup_Select.h and modify line 29:
    #include <Mitsubishi_M64282FP_dashcam_TFT_eSPI_setup.h>           // Default setup is root library folder
- Compile your code and flash the .uf2 to your board (Arduino do that automatically once you've pressed the BOOTSEL button once).

# User manual

- Once the device is booted, it adapt the sensor exposure for 2-3 seconds then run in Display mode. Dipslay shows what the Mitsubishi sensor sends without recording anything. The green LED indicate the exposure time. It can go from 256 µs to 1 second depending on lighting conditions.
- To record data, press the pushbutton linked to GPIO13. It will automatically record images with a deadtime inbetween. This deadtime is set by just creating a file named config.txt with that deadtime value entered in ms (if no file, default deadtime is 2000 ms). There is no minimal value. The red LED indicates access to the SD card. By pressing pushbutton again, Display mode comes back.

It is mandatory to format the SD card in FAT32 with the maximum sector size possible. The access to the SD card is the bottle neck in Record mode.

# Minimal parts

# Pinout

# Showcase



# Mitsubishi-M64282FP-dashcam

A SD card based recorder for the Mitsubishi M64282FP, sensor of the Game Boy Camera, for the Raspberry Pi Pico. The code was developped under the Arduino IDE with the Earle F. Philhower Raspberry Pi Pico Arduino core. TFT display is driven with the Bodmer TFT_eSPI library. The code originates from an [Arduino version](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor) that requires a PC. This project does not require a Game Boy Camera PCB, the Pi Pico directly drives the camera sensor and returns 8 bits images.

![showcase](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Showcase.png)

# Multi-systems installation guide

- Install the last [Arduino IDE](https://www.arduino.cc/en/software)
- Install the [Earle F. Philhower Raspberry Pi Pico Arduino core for Arduino IDE](https://github.com/earlephilhower/arduino-pico) via the Arduino Board manager (see [installation guide](https://github.com/earlephilhower/arduino-pico#installing-via-arduino-boards-manager)).
- Install the Benoit Blanchon [Arduino JSON library](https://github.com/bblanchon/ArduinoJson) via the Arduino library manager.
- Import or install the Bodmer [TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI) via the Arduino library manager.
- Locate the TFT_eSPI library: **\Arduino\libraries\TFT_eSPI** folder in your Arduino libraries
    
- copy the [Mitsubishi_M64282FP_dashcam_TFT_eSPI_setup.h](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Mitsubishi_M64282FP_dashcam_TFT_eSPI_setup.h) configuration file for the TFT display in this folder.
- edit the User_Setup_Select.h and modify line 29:

    **#include <Mitsubishi_M64282FP_dashcam_TFT_eSPI_setup.h> // Default setup is root library folder**
    
- Compile your code and flash the .uf2 to your board (Arduino IDE does that automatically once you've pressed the BOOTSEL button once).

# Basic user manual

- Once the device is booted, it adapts the sensor exposure for 2-3 seconds then run in **Display mode**. **Dipslay mode** shows what the Mitsubishi sensor sends without recording anything. The green LED indicates the exposure time. It can go from approx 1 ms to 1 second depending on lighting conditions. The red LED indicates access to the SD card for recording. 
- To shift to **Recording mode**, press the pushbutton linked to GPIO13. It will either automatically record BMP images with a deadtime inbetween ("Timelapse mode" activated), or act as a regular camera ("Timelapse mode" deactivated). Shifting from one mode to the other is made by pushing the button linked to GPIO22. 
- In **Timelapse mode**, the deadtime between pictures is set by just by modifying the [config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json) (value entered in ms, if no file, default deadtime is 2000 ms). During each recording session, images are stored in a different folder.
- In **Regular camera mode**, pressing the button just records one image in the **/camera** folder. If the button stays pushed, images are recorded continuously with a debouncing delay of 250 ms. It acts more or less as a burst mode.

To activate **HDR mode** or **Dithering mode** , simply push the corresponding pushbuttons linked to GPIO20 and GPIO21.

**HDR mode** take several images from -1EV to +1EV and make an average of them before recording. This increases the dynamic of the sensor. The list of exposures can be modified in the [config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json).

**Dithering mode** mimicks the dithering process of a Game Boy Camera with 4x4 derived Bayer matrices. Dithering matrices [generated online](https://herrzatacke.github.io/dither-pattern-gen/) can be copied in the **[config.h](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Mitsubishi_M64282FP_dashcam/config.h)** or  **[config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json)** file. **Config.json, if it exists, has priority on config.h.** It is not a necessary file but it increases available options.

It is mandatory to format the SD card in FAT32 and it is better to use the maximum sector size possible to speed up writing and avoid stalling. The access to the SD card is indeed the bottleneck in Recording mode.

Additionally, you can address other features by entering them in the [config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json):
- **Night Mode** allows automatically downclocking the device in case the exposure registers reaches their theoretical maximal value (0xFFFF). This is usefull to do light painting from example, the initial sensor being unable to expose mote than 1 second. Here there is no limit.
- **Fixed exposure** allows bypassing the autoexposure algorithm and fixing a value, useful for astrophotography where auto-exposure performs poorly. In this case you have to enter the exposure time or delay (between 0x0030 - 0.768 ms and 0xFFFF - 1.044 second) and the clock divider (a multiplier for the exposure time, stay at 1 if you do not know what to do.)
- **Pretty border mode** generates 160x144 images with a fancy border like the Game Boy Camera. You can make your own with the tools provided to generate C-files.
- **2D enhancement mode** activates (default state) or removes the 2D image enhancement processed by the sensor. The raw images (without 2D enhancement) looks blurry but they are not: this is the weak native resolution of the sensor that creates this feeling.

Options are cumulatives, it is for example possible to record dithered HDR images at fixed exposure without display in night mode. Yes, it would be a mess.

# Example of test images with a stuffed fox
![comparison](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Comparison.png)

# Advanced options

These options are available by modifying #defines in the **[config.h](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Mitsubishi_M64282FP_dashcam/config.h)** file:
- **USE_SERIAL** outputs 8 bits image data to the serial in ASCII. May be usefull for a third party program to get the data out of the sensor.
- **USE_TFT** allows compiling the code without display options for sneak attacks ! It still records on SD card however.
- **USE_SD**. Well, if you refuse to record your art because art is ephemeral. Joke part, useful to boot without SD card module attached for debugging purposes. 
- **USE_EXTREME_OVERCLOCK** allows multiplying the clock frequency by a factor of 2. Use at your own risks.
- **USE_SNEAK_MODE** disable LEDS in recording mode.

# Minimal parts

**Powered by the Pi Pico USB port**
- an Arduino Pi Pico. [Fancy purple Chinese clones](https://fr.aliexpress.com/item/1005003928558306.html) are OK (this is still the genuine RP2040 chip) as long as you do not care that the pinout is completely baroque.
- a [1.8 TFT SPI 128x160 display](https://fr.aliexpress.com/item/1005004536839797.html). Note that it is pixel perfect with the sensor.
- Two [4 gates bidirectionnal level shifters](https://fr.aliexpress.com/item/1005004560297038.html).
- a way to connect the sensor without destroying the original cable like a slighly butchered [JST ZH1.5MM 9 Pin](https://fr.aliexpress.com/item/32920487056.html) connector or an opened [replacement cable](https://www.digikey.com/en/products/base-product/jst-sales-america-inc/455/A09ZR09Z/588181) (choose B model).
- a [8x12 cm prototype board](https://fr.aliexpress.com/item/1005001636510673.html) while waiting for a real PCB.
- 2 LEDs (red and green) and two resistors of 200-500 Ohms.
- 5 [push buttons whatever their size or height](https://fr.aliexpress.com/item/1005003251295065.html).
- a [microswitch](https://fr.aliexpress.com/item/1005003938856402.html) to cut the display backlight which draws more current (30 mA) than the Pi Pico (25 mA) itself, for saving battery in case of long timelapses for example.

**Powered with internal lithium battery**
- an [USB breakout board of any kind](https://fr.aliexpress.com/item/4000385426649.html).
- a [DD05CVSA charger unit](https://fr.aliexpress.com/item/1005003537981780.html).
- any LiPo stolen in any electronic toy. The device draws nothing, even a tiny 200 mA.h is enough to play with the device outside for hours.
- another [microswitch](https://fr.aliexpress.com/item/1005003938856402.html) to cut the circuit when off or for flashing the Pi Pico.

The whole dashcam device requires a +5V line to drive the sensor and cannot unfortunately be powered by the VSYS pin only. If you're in a nerdy day you normally have all of the parts somewhere in the drawers. From scratch all parts will cost you about 15€.

# Pinout (yes it's a mess, I know)
![pinout](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Pinout_2.png)

RESET button is not mandatory but very practical. The device should normally never crash but if the sensor is deconnected, it may freeze the loop as the code wait some some response of the sensor. So reseating cable and RESET is the fix. There is honestly no room for other features without using the ADC pins.

# Connection with the sensor
![cable pinout](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Cable_pinout.png)

Hey ?! It's all empty in the shell apart from that crappy connector at the end ? Yes, you can trash the main camera PCB, or better give it to me to avoid dispersal of heavy metals, I will recycle it well, don't worry. The camera PCB is not required is this mod.

# Some technical thought for free

According to [internal Mitsubishi source](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/Yerazunis%20(1999)%20An%20Inexpensive%2C%20All%20Solid-state%20Video%20and%20Data%20Recorder%20for%20Accident%20Reconstruction.pdf), the use of the M64282FP artificial retina for dashcam application was assessed in 1999. They recommend using the [MAX153 flash ADC](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/MAX153%20-%201Msps%2C%20%C2%B5P-Compatible%2C8-Bit%20ADC%20with%201%C2%B5A%20Power-Down.pdf) to convert analog signal fast enough for a live (5 fps !) rendering and recording of images. The MAC-GBD, mapper of the Game Boy Camera, [embeds a flash ADC](https://github.com/Raphael-Boichot/Game-Boy-chips-decapping-project#game-boy-camera-mac-gbd-mapper) too on its chip. The probability is thin but not zero that the MAC-GBD flash ADC would be simply a MAX153 adapted for this custom mapper.

In other words, it should perfectly be possible to interface a MAX153 with the Raspberry Pi Pico (or any other µC) to fasten the analog->digital conversion. The Pi Pico can theoretically translate data at 500 ksamples/s (on this project, it reaches 400 ksamples/s) and the MAX153 1Msamples/s. A factor of two is achievable so.


# Acknowledgments
- [Game Boy Camera Club](https://disboard.org/fr/server/568464159050694666) on Discord for the hype and help on all new projects.
- [Rafael Zenaro](https://github.com/zenaro147) because I stole lots of code from the [NeoGB Printer project](https://github.com/zenaro147/NeoGB-Printer).
- [Andreas Hahn](https://github.com/HerrZatacke) for the fruitful technical dicussions and for the [dithering pattern generator](https://herrzatacke.github.io/dither-pattern-gen/).
- [Michael Shimniok](https://github.com/shimniok/avr-gameboy-cam) for the Arduino code I started from.

# To do (or not)
- attach buttons to interrupts to increase reactivity.
- a decent PCB because all that shit would probably enter a regular Game Boy shell at certain point.
- a decent schematic to cure my addiction to MS Powerpoint.
- a live analog recorder based on this project to interface with a Game Boy Camera sensor in real use with its mapper connected.

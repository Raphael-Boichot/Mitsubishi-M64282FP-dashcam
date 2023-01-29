# Mitsubishi-M64282FP-dashcam

A SD card based recorder for the Mitsubishi M64282FP, sensor of the Game Boy Camera, for the Raspberry Pi Pico. The code was developped under the Arduino IDE with the Earle F. Philhower Raspberry Pi Pico Arduino core. TFT Display driven with the Bodmer TFT_eSPI library. The code originates from an [Arduino version](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor) that requires a PC. This project does not require a Game Boy Camera PCB, the Pi Pico directly drives the camera sensor and returns 8 bits images.

![showcase](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Showcase.png)

# Multi-systems installation guide

- Install the last [Arduino IDE](https://www.arduino.cc/en/software)
- Install the [Earle F. Philhower Raspberry Pi Pico Arduino core for Arduino IDE](https://github.com/earlephilhower/arduino-pico) via the Arduino Board manager (see [installation guide](https://github.com/earlephilhower/arduino-pico#installing-via-arduino-boards-manager)).
- Import or install the [Bodmer TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI).
- Locate the TFT_eSPI library: **\Arduino\libraries\TFT_eSPI** folder in your Arduino libraries
    
- copy the [Mitsubishi_M64282FP_dashcam_TFT_eSPI_setup.h](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Mitsubishi_M64282FP_dashcam_TFT_eSPI_setup.h) configuration file for the TFT display in this folder.
- edit the User_Setup_Select.h and modify line 29:

    **#include <Mitsubishi_M64282FP_dashcam_TFT_eSPI_setup.h> // Default setup is root library folder**
    
- Compile your code and flash the .uf2 to your board (Arduino IDE does that automatically once you've pressed the BOOTSEL button once).

# User manual

- Once the device is booted, it adapts the sensor exposure for 2-3 seconds then run in **Display mode**. **Dipslay mode** shows what the Mitsubishi sensor sends without recording anything. The green LED indicate the exposure time. It can go from 256 µs to 1 second depending on lighting conditions.
- To shift to **Recording mode**, press the pushbutton linked to GPIO13. It will automatically record BMP images with a deadtime inbetween. This deadtime is set by just dropping a file named [config.txt](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/config.txt) at the root of SD card with that deadtime value entered in ms (if no file, default deadtime is 2000 ms). There is no minimal value, the SD can capture each frame if needed. The red LED indicates access to the SD card for recording. By pressing pushbutton again, **Display mode** comes back.
- To activate **HDR mode**, simply push the HDR pushbutton linked to GPIO20 during **Display mode** (it is decativated during Recording mode).

It is mandatory to format the SD card in FAT32 and it is better to use the maximum sector size possible to speed up writing. The access to the SD card is indeed the bottleneck in Record mode.

# Example of image output with a stuffed fox as main NPC
![comparison](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Comparison.png)

# Minimal parts

**Powered by the Pi Pico USB port**
- An Arduino Pi Pico. [Fancy purple Chinese clones](https://fr.aliexpress.com/item/1005003928558306.html) are OK (this is still the genuine RP2040 chip) as long as you do not care that the pinout is completely baroque.
- a [1.8 TFT SPI 128x160 display](https://fr.aliexpress.com/item/1005004536839797.html).
- Two [4 gates bidirectionnal level shifters](https://fr.aliexpress.com/item/1005004560297038.html).
- A way to connect the sensor without destroying the original cable like a [JST ZH1.5MM 9 Pin](https://fr.aliexpress.com/item/32920487056.html) or a butchered [replacement cable](https://www.digikey.com/en/products/base-product/jst-sales-america-inc/455/A09ZR09Z/588181) (choose B model).
- A [8x12 cm prototype board](https://fr.aliexpress.com/item/1005001636510673.html) while waiting for a real PCB.
- 2 LEDs (red and green) and two resistors of 200-500 Ohms.
- 3 [push buttons whatever their size or height](https://fr.aliexpress.com/item/1005003251295065.html).

**Powered with internal lithium battery**
- an [USB breakout board of any kind](https://fr.aliexpress.com/item/4000385426649.html).
- a [DD05CVSA charger unit](https://fr.aliexpress.com/item/1005003537981780.html).
- Any LiPo stolen in any electronic toy. The device draws nothing, even a 200 mA.h is enough to play with the device outside for hours.
- a [microswitch](https://fr.aliexpress.com/item/1005003938856402.html) to cut the circuit when off or for flashing the Pi Pico.

The whole dashcam device requires a +5V line to drive the sensor and cannot unfortunately be powered by the VSYS pin only.

# Pinout
![pinout](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Pinout.png)

# Connection with the sensor
![cable pinout](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Cable_pinout.png)

Hey ?! It's all empty in the shell apart from that crappy connector at the end ? Yes, you can trash the main camera PCB, or better give it to me to avoid dispersal of heavy metals, I will recycle it well, don't worry. The camera PCB is not required is this mod.

# acknowledgments
- [Game Boy Camera Club](https://disboard.org/fr/server/568464159050694666) on Discord for the hype and help on all new projects.
- [Rafael Zenaro](https://github.com/zenaro147) because I stole lots of code from the [NeoGB Printer project](https://github.com/zenaro147/NeoGB-Printer).
- [Andreas Hahn](https://github.com/HerrZatacke) for the fruitful technical dicussions.

# To do (or not)

- a decent PCB because all that shit could enter a Game Boy shell.
- a decent schematic to cure my addiction to MS Paint.

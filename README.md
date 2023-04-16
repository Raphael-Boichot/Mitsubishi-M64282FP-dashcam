# The DASHBOY CAMERA

A SD card based recorder for the Mitsubishi M64282FP, sensor of the Game Boy Camera. The code was developped for Raspberry Pi Pico with the Earle F. Philhower RP2040 core. TFT display is driven with the Bodmer TFT_eSPI library. The code originates from an [Arduino version](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor) that requires a PC. This project does not require a Game Boy Camera PCB, the Pi Pico directly drives the camera sensor and returns 8 bits images.

# Picture of the device

![showcase](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/PCB_version_2.0.png)

# Multi-systems installation guide

- Install the last [Arduino IDE](https://www.arduino.cc/en/software)
- Install the [Earle F. Philhower Raspberry Pi Pico Arduino core for Arduino IDE](https://github.com/earlephilhower/arduino-pico) via the Arduino Board manager (see [installation guide](https://github.com/earlephilhower/arduino-pico#installing-via-arduino-boards-manager)).
- Install the Benoit Blanchon [Arduino JSON library](https://github.com/bblanchon/ArduinoJson) via the Arduino library manager.
- Import or install the Bodmer [TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI) via the Arduino library manager.
- Locate the TFT_eSPI library: **\Arduino\libraries\TFT_eSPI** folder in your Arduino libraries
    
- copy the [configuration file](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/128x160%20THT%20version/128x160_Dashboy_Camera/128x160_TFT%20Setup) for the TFT display in this folder.
- edit the User_Setup_Select.h and modify line 29:

    **#include <Mitsubishi_M64282FP_dashcam_TFT_eSPI_setup.h> // Default setup is root library folder**
    
- Compile your code and flash the .uf2 to your board (Arduino IDE does that automatically once you've pressed the BOOTSEL button once).

OR

- Just flash the last built version to your Pi Pico board.

# Basic user manual

The **Dashboy Camera** uses a [configuration file](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json) that must be placed at the root of SD card which has priority on the internal default parameters. The device cannot boot without a working sensor and a working SD card. Status of sensor, SD card and configuration file are indicated on the splashscreen.

- Once the device is booted, it adapts the sensor exposure for 2-3 seconds then runs in **Display mode**. Display mode does nothing else than showing on display what the Mitsubishi sensor sees at the maximum allowed refresh rate. The green LED indicates the exposure time. It can go from approx 1 ms to 1 second depending on lighting conditions. The red LED indicates access to the SD card for recording (or error at boot). 
- There are two main modes: **Regular Camera mode** to take picture one by one like any camera and **Timelapse mode** to automatically take pictures with a delay inbetween. Shifting from one mode to the other is made by pushing the **TIMELAPSE** button. It rolls over a list of 7 exposure delays, the 8th being the return to regular camera mode. In the json file, delays are entered in ms, a delay of -1 indicates a return to regular camera mode. **Keep the total length of the table at 8 entries !** 
- In **Timelapse mode**, each recording session uses a different folder to store images. Timelapse mode has virtually no ending, it can go as long as the SD card is not full. If delay between images is more than 10 s (10000 ms), the device enters a sleeping loop every second and wake up only briefly to accomodate exposure. The happy consequence is that electrical consumption falls to almost zero. The drawback is that the interface is less responsive. You typically have to push buttons for about one second to get a response.
- In **Regular camera mode**, pressing the button just records one image in the **/camera** folder. If the button stays pushed, images are recorded continuously with a debouncing delay of 250 ms. It acts more or less as a burst mode.

To activate **HDR mode** or **Dithering mode**, simply push the corresponding pushbuttons.

**HDR mode** take several images from -1EV to +1EV and make an average of them before recording. This increases the dynamic of the sensor. The list of exposures can be modified in the [config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json). **Keep the total length of the table at 8 entries !**

**Dithering mode** mimicks the dithering process of a Game Boy Camera with 4x4 derived Bayer matrices. Dithering matrices [generated online](https://herrzatacke.github.io/dither-pattern-gen/) can be copied in the **[config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json)** file. **Config.json, if it exists, has always priority on config.h.** Json is not a necessary file but it increases available options.

Finally, you can **lock exposure time from the current best one** in any mode by pressing the **LOCK_EX** push button. It activates the internal board LED when it is on.

It is mandatory to format the SD card in FAT32 and it is better to use the maximum sector size possible to speed up writing and avoid stalling. The access to the SD card is indeed the bottleneck in Recording mode.

# Advanced user manual

Additionally, you can address other cool features by entering them in the [config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json):
- **Raw recording mode** available in **time lapse mode only**, records sensor data as a single file in the **/movie** folder. It needs a separate tool for extracting an animation ([provided here](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Tools/Movie_and_Gif_Maker_from_raw_data.m)). The 32 first bytes are keywords, image sizes and registers, the following are raw data for one image in reading order, easy to recover with any other tool ! This mode allows recording at a solid 4 fps for long time with good light conditions. It is disabled in **Regular camera mode** (command is just ignored).
- **Night Mode** allows automatically downclocking the device in case the exposure registers reaches their theoretical maximal value (0xFFFF). This is usefull to do light painting from example, the initial sensor being unable to expose mote than 1 second. Here there is no limit.
- **Fixed exposure mode** allows bypassing the autoexposure algorithm and fixing a value, useful for astrophotography where auto-exposure performs poorly. In this case you have to enter the exposure time or delay (between 0x0030 - 0.768 ms and 0xFFFF - 1.044 second) and the clock divider (a multiplier for the exposure time, stay at 1 if you do not know what to do.)
- **Pretty border mode** generates 160x144 images with a fancy border like the Game Boy Camera. You can make your own with the tools provided to generate C-files.
- **2D enhancement mode** activates (default state) or removes the 2D image enhancement processed by the sensor. The raw images (without 2D enhancement) looks blurry but they are not: this is the weak native resolution of the sensor that creates this feeling.
- **Game Boy Camera mode** drives the DashBoy Camera exactly as the Game Boy Camera does, using the same registers and the same dithering matrices in default constrast. Images made in this mode are pixel identical to images produces with a Game Boy Camera in the same lightning condition if dithering is activated. When set to 0, this triggers a simplier "recipe" with fixed registers and fixed dithering matrix.

# Some pictures made with the device

![pictures](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/Examples.png)

## Building the standard 128x160 through-hole version (for beginners in soldering)

## Required parts

- An [Arduino Pi Pico](https://fr.aliexpress.com/item/1005003928558306.html). **Be sure to select the regular/original green board with the official pinout and castellated holes.**
- A [1.8 TFT SPI 128x160 display](https://fr.aliexpress.com/item/1005004536839797.html). Note that it is pixel perfect with the sensor.
- Two [4 gates bidirectionnal level shifters](https://fr.aliexpress.com/item/1005004560297038.html).
- Some [9 pins JST ZH1.5MM connector](https://fr.aliexpress.com/item/32920487056.html).
- Some [9 pins, double head, 10 cm, JST ZH1.5MM cables](https://fr.aliexpress.com/item/1005004501408268.html).
- Some [male and female pin headers](https://fr.aliexpress.com/item/1005002577212594.html).
- A [DC-DC 5 volts regulator](https://fr.aliexpress.com/item/32813355879.html).
- A [2xAA battery holder with cover](https://fr.aliexpress.com/item/1005004651271276.html). Do not take opened ones as they are generally crap.
- 2 regular 5 mm LEDs (red and green) and two through hole resistors of 250 to 1000 Ohms (low value = high brighness).
- A 10 volts through hole capacitor of 500 to 1000 microfarads (high value = more stable 5 volts on the board).
- 6 [6x6 push buttons whatever their height](https://fr.aliexpress.com/item/1005003938244847.html).
- 2 [microswitches SS-12D00G](https://fr.aliexpress.com/item/1005003938856402.html) to cut the main power and the display backlight which draws more current (30 mA) than the Pi Pico (25 mA) itself, for saving battery in case of long timelapses for example.

The device is meant to be used with NiMH batteries. I very dislike lithium batteries because they are dangerous to store, not recycled and non generic at all.

## PCB and connection with the sensor

PCB for left and rigth handed users are available in the [PCB](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/128x160%20THT%20version/128x160_PCB%20and%20schematic) folder of the project. To connect the sensor to the board, use a female JST connector with bended pins in order to inverse the gender of the JST cable, or simply order the [breakout PCB](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/128x160%20THT%20version/128x160_PCB%20and%20schematic/Sensor%20to%20PCB%20adapter%20board). Be carefull with the polarity: both faces of the cables must be the same (camera sensor cable side and extension cable side, see following images). This situation corresponds also to the INPUT<->NORMAL OUT of the breakout board if you order the extension cables given in **Required parts** section.

![connections](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/Ribbon_connection.png)

PCBs can be ordered at [JLCPCB](https://jlcpcb.com/) by simply uploading the gerber zip container to their site. JLCPCB has the advantage to be cheap and clean with the VAT for EU customers. Order HASL finish with 1.6 mm thickness for the two boards (default option). PCB designs and schematics can be edited with [EasyDA](https://easyeda.com/fr).

## Building instructions

- Solder the Pi Pico first: **it must be mandatorily soldered on the castellated holes, without pin headers or spacer, directly on the PCB**
- Solder the lower parts next: diodes, resistors, level shifters, push buttons, JST connector, microswitches, capacitor and DC-DC converter. Levels shifters and DC-DC converters can be soldered with male pin headers.
- Solder the display: **ensure to have enough clearance to slide the SD card out of the display above the Pi Pico**. Best is to use female pin headers to secure the distance between display and main PCB but the default pins are long enough if you solder them just showing out of the back PCB surface.
- Trim the pin connectors from the back side of PCB.
- At this step, flash the Pi Pico and verify that the device boots and the display works. It should be stuck to an error screen with the red LED falshing, this is normal as the device needs SD card and a sensor to go further.
- Solder the JST connectors to the [tiny adapter board](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/128x160%20THT%20version/128x160_PCB%20and%20schematic/Sensor%20to%20PCB%20adapter%20board/Tiny%20adapter%20board.png) (INPUT and NORMAL OUT if you use a cable similar to the default sensor one linked in **Parts**) or just bend straight the pins of a female JST connector and plug it into the JST extension cable to inverse its gender.
- Trim the AA battery holder ON/OFF switch and place it in ON position, solder the battery terminals.
- Glue the Battery holder behing the pushbuttons (it will act as a grip) and the camera shell behind the display. Use double sided tape or dots of hotglue in order to allow reversing the mod and get back your camera shell if necessary.
- Connect the sensor to the extension cable or adapter board (beware of the polarity), place fresh AA batteries and enjoy your Dashboy Camera !

## Building the mini 240x240 surface mount version (requires some skill in soldering)

## Required parts
To do

## Building instructions
To do

# Some usefull informations for Game Boy Camera nerds

- The sensor has intrinsically many visual artifacts (horizontal and vertical lines, image echoes at certain exposures, etc.). None of them is specifically due to the Pi Pico. They were all observed with a Game Boy Camera in the same conditions of light.
- The exposure register is limited to 0x0030 despite the camera being able to go to 0x0001. This is advised in the sensor datasheet and the reason is that below this value, the image artifacts becomes so intense that they interfere with any auto-exposure algorithm based on pixel intensity. Technically, this creates exposure jitters and unleasant behavior. Game Boy Camera mode have a range of exposure extended to 0x0010 by playing on other registers at the same time.
- The strategy used in Game Boy Camera mode and the auto-exposure algorithm have been sourced from real data logging on a regular Game Boy Camera. Source files can be found in this [other repository](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor/tree/main/Research%20on%20real%20Camera).
- Close up images of the sensor and the MAC-GBD decapped can be found [here](https://github.com/Raphael-Boichot/Game-Boy-chips-decapping-project). The sensor presents masked pixels which always return the maximal possible voltage for a given set of registers. I used them to calibrate the code but their signal is just dismissed in the current implementation. They occupy the lines 123 to 128 of a 128x128 image.
- The role of each registers is quite well explained [here](https://github.com/untoxa/gb-photo). The Mitsubishi M64282FP datasheet is notorious for being crap so many informations were discovered by trial and error and educated guess.

# Some thought for free

According to [internal Mitsubishi source](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/Yerazunis%20(1999)%20An%20Inexpensive%2C%20All%20Solid-state%20Video%20and%20Data%20Recorder%20for%20Accident%20Reconstruction.pdf), the use of the M64282FP artificial retina for dashcam application was assessed in 1999. They recommend using the [MAX153 flash ADC](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/MAX153%20-%201Msps%2C%20%C2%B5P-Compatible%2C8-Bit%20ADC%20with%201%C2%B5A%20Power-Down.pdf) to convert analog signal fast enough for a live (5 fps !) rendering and recording of images. The MAC-GBD, mapper of the Game Boy Camera, [embeds a flash ADC](https://github.com/Raphael-Boichot/Game-Boy-chips-decapping-project#game-boy-camera-mac-gbd-mapper) too on its chip. The probability is thin but not zero that the MAC-GBD flash ADC would be simply a MAX153 adapted for this custom mapper. A copy of this experiment [is proposed here](https://github.com/Raphael-Boichot/Game-Boy-camera-sniffer). And yes it works.

# Kind warning

The code and current design come as it. If you're not happy with the current hardware, the PCB EasyDA design or the Arduino IDE, build your own, the licence allows it ! Push request with tested and working improvements are of course still welcomed.

# To do

- calculate the low and high light dithering arrays to put in the json
- Remove the auto-contrast for the Game Boy Camera Strategy.

# Acknowledgments

- [Game Boy Camera Club](https://disboard.org/fr/server/568464159050694666) on Discord for the hype and help on all new projects. You can discuss with the authors here !
- [Rafael Zenaro](https://github.com/zenaro147) because I stole lots of code from the [NeoGB Printer project](https://github.com/zenaro147/NeoGB-Printer).
- [Andreas Hahn](https://github.com/HerrZatacke) for the fruitful technical dicussions and for the [dithering pattern generator](https://herrzatacke.github.io/dither-pattern-gen/).
- [Michael Shimniok](https://github.com/shimniok/avr-gameboy-cam) and [Laurent Saint-Marcel](https://github.com/BackupGGCode/avr-gameboy-cam) for the Arduino code I started from (initial [source](http://sophiateam.undrgnd.free.fr/microcontroller/camera/) here).

# The DASHBOY CAMERA

A SD card based recorder for the Mitsubishi M64282FP, sensor of the Game Boy Camera, for the Raspberry Pi Pico. The code was developped under the Arduino IDE with the Earle F. Philhower Raspberry Pi Pico Arduino core. TFT display is driven with the Bodmer TFT_eSPI library. The code originates from an [Arduino version](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor) that requires a PC. This project does not require a Game Boy Camera PCB, the Pi Pico directly drives the camera sensor and returns 8 bits images.

# Picture of the device

![showcase](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/PCB_version_2.0.png)

# Multi-systems installation guide

- Install the last [Arduino IDE](https://www.arduino.cc/en/software)
- Install the [Earle F. Philhower Raspberry Pi Pico Arduino core for Arduino IDE](https://github.com/earlephilhower/arduino-pico) via the Arduino Board manager (see [installation guide](https://github.com/earlephilhower/arduino-pico#installing-via-arduino-boards-manager)).
- Install the Benoit Blanchon [Arduino JSON library](https://github.com/bblanchon/ArduinoJson) via the Arduino library manager.
- Import or install the Bodmer [TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI) via the Arduino library manager.
- Locate the TFT_eSPI library: **\Arduino\libraries\TFT_eSPI** folder in your Arduino libraries
    
- copy the [configuration file](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/128x160_Dashboy_Camera/128x160_TFT%20Setup) for the TFT display in this folder.
- edit the User_Setup_Select.h and modify line 29:

    **#include <Mitsubishi_M64282FP_dashcam_TFT_eSPI_setup.h> // Default setup is root library folder**
    
- Compile your code and flash the .uf2 to your board (Arduino IDE does that automatically once you've pressed the BOOTSEL button once).

# Basic user manual

First of all, the device is basically a timelapse machine. The regular camera mode is just coded as a timelapse of one image. It uses a configuration file placed at the root of SD card which has priority on the internal default parameters. The device cannot boot without a working sensor and a working SD card. Status of sensor, SD card and configuration file are indicated on the splashscreen.

- Once the device is booted, it adapts the sensor exposure for 2-3 seconds then run in **Display mode**. **Dipslay mode** do nothing else than showing on display what the Mitsubishi sensor sees. The green LED indicates the exposure time. It can go from approx 1 ms to 1 second depending on lighting conditions. The red LED indicates access to the SD card for recording. 
- To shift to **Recording mode**, press the PUSH button. It will either automatically record BMP images with a deadtime inbetween ("Timelapse mode" activated), or act as a regular camera ("Timelapse mode" deactivated). Shifting from one mode to the other is made by pushing the TIMELAPSE button. 
- In **Timelapse mode**, the deadtime between pictures is set by just by modifying the [config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json) (value entered in ms, if no file, default deadtime is 2000 ms). During each recording session, images are stored in a different folder. Timelapse mode has virtually no ending, it can go as long as the SD card is not full.
- In **Regular camera mode**, pressing the button just records one image in the **/camera** folder. If the button stays pushed, images are recorded continuously with a debouncing delay of 250 ms. It acts more or less as a burst mode.

To activate **HDR mode** or **Dithering mode**, simply push the corresponding pushbuttons.

**HDR mode** take several images from -1EV to +1EV and make an average of them before recording. This increases the dynamic of the sensor. The list of exposures can be modified in the [config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json).

**Dithering mode** mimicks the dithering process of a Game Boy Camera with 4x4 derived Bayer matrices. Dithering matrices [generated online](https://herrzatacke.github.io/dither-pattern-gen/) can be copied in the **[config.h](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/128x160_Dashboy_Camera/config.h)** or  **[config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json)** file. **Config.json, if it exists, has priority on config.h.** It is not a necessary file but it increases available options.

Finally, you can **lock exposure** in any mode by pressing the LOCK_EX push button. It activates the board LED when it is on.

It is mandatory to format the SD card in FAT32 and it is better to use the maximum sector size possible to speed up writing and avoid stalling. The access to the SD card is indeed the bottleneck in Recording mode.

Additionally, you can address other features by entering them in the [config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json):
- **Night Mode** allows automatically downclocking the device in case the exposure registers reaches their theoretical maximal value (0xFFFF). This is usefull to do light painting from example, the initial sensor being unable to expose mote than 1 second. Here there is no limit.
- **Fixed exposure** allows bypassing the autoexposure algorithm and fixing a value, useful for astrophotography where auto-exposure performs poorly. In this case you have to enter the exposure time or delay (between 0x0030 - 0.768 ms and 0xFFFF - 1.044 second) and the clock divider (a multiplier for the exposure time, stay at 1 if you do not know what to do.)
- **Pretty border mode** generates 160x144 images with a fancy border like the Game Boy Camera. You can make your own with the tools provided to generate C-files.
- **2D enhancement mode** activates (default state) or removes the 2D image enhancement processed by the sensor. The raw images (without 2D enhancement) looks blurry but they are not: this is the weak native resolution of the sensor that creates this feeling.
- **Raw recording mode** available in **time lapse mode only**, records sensor data as a single file in the **/movie** folder. It needs a separate tool for extracting an animation ([provided here](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Tools/Movie_and_Gif_Maker_from_raw_data.m)). The 32 first bytes are keywords, image sizes and registers, the following are raw data for one image in reading order, easy to recover with any other tool ! This mode allows recording at a solid 4 fps for long time with good light conditions. It is disabled in **Regular camera mode** (command is just ignored).

Options are cumulatives, it is for example possible to record dithered HDR images at fixed exposure without display in night mode. Yes, it would be a mess.

# Advanced options

These options are available only by modifying the #defines in the **[config.h](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/128x160_Dashboy_Camera/config.h)** file, which requires compiling the code:
- **USE_SERIAL** outputs 8 bits image data to the serial in ASCII. May be usefull for a third party program to get the data out of the sensor.
- **USE_TFT** allows compiling the code without display options for sneak attacks ! It still records on SD card however.
- **USE_SD**. Well, if you refuse to record your art because art is ephemeral. Joke part, useful to boot without SD card module attached for debugging purposes. 
- **USE_SNEAK_MODE** disable LEDS in recording mode.

# Minimal parts

- An [Arduino Pi Pico](https://fr.aliexpress.com/item/1005003928558306.html). **Be sure to select the regular green board with the official pinout and not the fancy purple variations.**
- A [1.8 TFT SPI 128x160 display](https://fr.aliexpress.com/item/1005004536839797.html). Note that it is pixel perfect with the sensor.
- Two [4 gates bidirectionnal level shifters](https://fr.aliexpress.com/item/1005004560297038.html).
- Some [9 pins JST ZH1.5MM connector](https://fr.aliexpress.com/item/32920487056.html).
- Some [9 pins, double head, 10 cm, JST ZH1.5MM cables](https://fr.aliexpress.com/item/1005004501408268.html).
- Some [male and female pin headers](https://fr.aliexpress.com/item/1005002577212594.html).
- A [DC-DC 5 volts regulator](https://fr.aliexpress.com/item/32813355879.html).
- A [2xAA battery holder with cover](https://fr.aliexpress.com/item/1005004651271276.html). Do not take opened ones as they are generally crap.
- 2 LEDs (red and green) and two resistors of 200-500 Ohms.
- A 10 volts capacitor of approx 500-1000 microfarads.
- 6 [6x6 push buttons whatever their height](https://fr.aliexpress.com/item/1005003938244847.html).
- 2 [microswitches](https://fr.aliexpress.com/item/1005003938856402.html) to cut the main power and the display backlight which draws more current (30 mA) than the Pi Pico (25 mA) itself, for saving battery in case of long timelapses for example.

# PCB and connection with the sensor

PCB for left and rigth handed users are available in the [PCB]() folder of the project. To connect the sensor to the board, use a female JST connector with bended pins in order to inverse the gender of the JST cable, or simply order the [breakout PCB](). Be carefull with the polarity: both faces of the cables must be the same (camera sensor cable side and extension cable side, see following images).

PCBs can be ordered at [JLCPCB](https://jlcpcb.com/) by simply uploading the gerber zip container to their site. JLCPCB has the advantage to be cheap and clean with the VAT for Eu customers. Order HASL finish with 1.6 mm thickness for the two boards (default option).

# Building instructions

- Solder the lower parts: diodes, resistors, level shifters, push buttons, JST connector, microswitches, capacitor, Pi Pico and DC-DC converter. **Pi Pico must be mandatorily soldered at the castellated holes, without pin headers or spacer, directly on the PCB**, levels shifters and DC-DC converters can be soldered with male pin headers.
- Solder the display **by ensuring to have enough clearance to slide the SD card above the Pi Pico**. Best is to use female pin headers to secure the distance between display and main PCB but the default pins are long enough if you solder them just showing out of the back PCB surface.
- At this step, flash the Pi Pico and verify that the device boots and the display works.
- Solder the JST connectors to the tiny adapter board (INPUT and NORMAL OUT if you use a cable similar to the default sensor one) or just bend the pins of a female JST connector and plug it into the JST extension cable.
- Trim the AA battery holder ON/OFF switch and place it in On position, solder the battery terminals.
- Glue the Battery holder behing the pushbuttons (it will act as a grip) and the camera shell behind the display. Use double sided tape or hotglue in order to allow reversing the mod if necessary.
- Connect the sensor to the extension cable or adapter board (beware of the polarity), place fresh AA batteries and enjoy your Dashboy Camera !

# Link to the board with a spare JST connector and a JST extension cable

![connections](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/Ribbon_connection.png)

# Some technical thought for free

According to [internal Mitsubishi source](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/Yerazunis%20(1999)%20An%20Inexpensive%2C%20All%20Solid-state%20Video%20and%20Data%20Recorder%20for%20Accident%20Reconstruction.pdf), the use of the M64282FP artificial retina for dashcam application was assessed in 1999. They recommend using the [MAX153 flash ADC](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/MAX153%20-%201Msps%2C%20%C2%B5P-Compatible%2C8-Bit%20ADC%20with%201%C2%B5A%20Power-Down.pdf) to convert analog signal fast enough for a live (5 fps !) rendering and recording of images. The MAC-GBD, mapper of the Game Boy Camera, [embeds a flash ADC](https://github.com/Raphael-Boichot/Game-Boy-chips-decapping-project#game-boy-camera-mac-gbd-mapper) too on its chip. The probability is thin but not zero that the MAC-GBD flash ADC would be simply a MAX153 adapted for this custom mapper. A working copy of this experiment [is proposed here](https://github.com/Raphael-Boichot/Game-Boy-camera-sniffer).

# Kind warning

The code and current design come as it. If you're not happy with the current hardware and the Arduino IDE, build your own, the licence allows it ! Push request with tested and working improvements are of course still welcomed.

# To do

- perfect emulation of the dithering strategy of the Game Boy Camera
- A Game Boy printer emulator based on this exact same hardware with fancy display during printing

# Acknowledgments

- [Game Boy Camera Club](https://disboard.org/fr/server/568464159050694666) on Discord for the hype and help on all new projects.
- [Rafael Zenaro](https://github.com/zenaro147) because I stole lots of code from the [NeoGB Printer project](https://github.com/zenaro147/NeoGB-Printer).
- [Andreas Hahn](https://github.com/HerrZatacke) for the fruitful technical dicussions and for the [dithering pattern generator](https://herrzatacke.github.io/dither-pattern-gen/).
- [Michael Shimniok](https://github.com/shimniok/avr-gameboy-cam) and [Laurent Saint-Marcel](https://github.com/BackupGGCode/avr-gameboy-cam) for the Arduino code I started from (initial [source](http://sophiateam.undrgnd.free.fr/microcontroller/camera/) here).

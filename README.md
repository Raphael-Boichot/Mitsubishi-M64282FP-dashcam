# The DASHBOY CAMERA
## A 8-bit digital camera made from the Mitsubishi M64282FP artificial retina and a Raspberry Pi Pico. 

**Media coverage:**
- **Hackaday: [A Game Boy Camera, Without The Game Boy](https://hackaday.com/2023/07/29/a-game-boy-camera-without-the-game-boy/)**
- **PetaPixel: [This DIY Game Boy Camera Ditches the Game Boy, Keeps the Camera](https://petapixel.com/2023/07/31/this-diy-game-boy-camera-ditches-the-game-boy-keeps-the-camera/)**
- **Hackster.io: [Turn a Game Boy Camera Into a Respectable Shooter](https://www.hackster.io/news/turn-a-game-boy-camera-into-a-respectable-shooter-63144660379f)**
- **pxl/mag: [Capture HDR and 256 Grayscale with Game Boy Camera](https://pxlmag.com/capture-hdr-and-256-grayscale-with-game-boy-camera#google_vignette)**
- **DIYPhotography: [The Game Boy Camera, minus the Game Boy, shoots 256 greyscale and HDR](https://www.diyphotography.net/the-game-boy-camera-minus-the-game-boy-shoots-256-greyscale-and-hdr/)**
- **TIME EXTENSION: [This Game Boy Camera Eradicates The Need For A Game Boy](https://www.timeextension.com/news/2023/07/this-game-boy-camera-eradicates-the-need-for-a-game-boy)**

The project fully emulates the sensor management strategy of the Game Boy Camera while completely bypassing the MAC-GBD mapper, which means you can record pixel perfect images without the limitation of storage space or the need for a Game Boy Printer emulator. The device can output dithered 2 bits per pixel images (like the Game Boy Camera) as well as smooth 8 bits per pixel images that take advantage of the full sensor bit-depth. It features time lapse recording, live recording, motion detection, High Dynamic Range, multi-exposure, night mode, Peak Focusing, Slit-scan photography, fancy borders and even more. 

**The code was developped for the Raspberry Pi Pico and the Arduino IDE**, using the [Earle F. Philhower RP2040 core](https://github.com/earlephilhower/arduino-pico). TFT display is driven with the [Bodmer TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI). The code originates from an [Arduino version](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor) that requires a PC. This project was preceded by a [regrettable attempt](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor/tree/main/ESP32_version_beta) to do the same with an ESP32 (good luck porting back the code to it with the same functions now). 

If you manage to touch the unobtainium [M64283FP](https://github.com/Raphael-Boichot/Play-with-the-Mitsubishi-M64283FP-sensor) sensor, which is probably a parent version of the [M64282FP](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor), it is fully compatible with the present device (but more or less with the Game Boy Camera, see [the full story](#some-random-informations-for-you-game-boy-camera-nerd-)). I was lucky enough to get some and study how to make them work smoothly.

You're of course free to modify this project as you wish, all sources are available here (all files for PCBs, all source codes, all documentation used). **The project was designed to be clean, cheap and easy to make** (requiring only cheap and very common parts, very basic soldering skills, no 3D printed parts, simple Arduino IDE environment and some hot glue). Fork it and play with it !

# Picture of the device (example of PCB V1.0)

![Dashboy Camera V1.0](Docs%20and%20research/Image%20files/PCB_version_1.0.png)

# Multi-systems installation guide

- **Just drop one of the [compiled uf2 files](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/Build) to the board in mass storage media mode (Connect the Pico to USB port with the BOOTSEL button pushed and release).**

Or if you can sustain moderate pain:

- Install the last [Arduino IDE](https://www.arduino.cc/en/software)
- Install the [Earle F. Philhower Raspberry Pi Pico Arduino core for Arduino IDE](https://github.com/earlephilhower/arduino-pico) via the Arduino Board manager (see [installation guide](https://github.com/earlephilhower/arduino-pico#installing-via-arduino-boards-manager)).
- Install the Benoit Blanchon [Arduino JSON library](https://github.com/bblanchon/ArduinoJson) via the Arduino library manager.
- Install the Bodmer [TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI) via the Arduino library manager.

Then:

- Locate the TFT_eSPI library: **\Arduino\libraries\TFT_eSPI** folder in your Arduino libraries
- copy the [configuration file](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/Codes/128x160%20THT%20version/128x160_Dashboy_Camera/128x160_TFT%20Setup) for the TFT display in this folder.
- edit the User_Setup_Select.h and modify line 29:

    **#include <Mitsubishi_M64282FP_dashcam_TFT_eSPI_setup.h> // Default setup is root library folder**
    
- Compile your code **with frequency set at CPU Speed: "133 MHz" and Optimize: "Optimize even more (-O3)"** (Menu Tools) and flash the .uf2 to your board (Arduino IDE does that automatically once you've pressed the BOOTSEL button once).
- Enjoy your brand new 0.015 Mpixels camera !

# Casual user manual (requirement: just pushing buttons)

The **Dashboy Camera** uses a [configuration file](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json) that must be placed at the root of SD card which has priority on the internal default parameters. The device cannot boot without a working sensor and a working SD card. Status of sensor, SD card and configuration file are indicated on the splashscreen.

- Once the device is booted, it adapts the sensor exposure for 2-3 seconds then runs in **Display mode**. Display mode does nothing else than showing on display what the Mitsubishi sensor sees at the maximum allowed refresh rate allowed by the device. The green LED indicates the exposure time. It can go from approx 250 microseconds to 1 second depending on lighting conditions. The red LED indicates access to the SD card for recording (or error at boot). 
- There are four main modes: **Regular Camera mode** to take picture one by one like any camera, **Motion sensor mode** that triggers pictures according to image change, **Timelapse mode** to automatically take pictures with a delay inbetween and **Slit-scan mode*** to perform slit-scan photography. Shifting from one mode to the other is made by pushing the **TIMELAPSE** button. It rolls over a list of exposure delays for time lapse and the three other modes. In the json configuration file, delays are entered in ms, a value of -1 indicates a return to **Regular camera mode**, a value of -2 indicates a shift to **Motion sensor mode** and a value of -3 or -4 triggers one of the two slit-scan modes. **Keep the total length of the table at 8 entries !** 
- In **Timelapse mode**, each recording session uses a different folder to store images. Timelapse mode has virtually no ending, it can go as long as the SD card is not full. The device enters a sleeping loop every 1/10 of the timelapse delay and wake up only briefly to accomodate exposure. The happy consequence is that electrical consumption falls to almost zero. The drawback is that the interface is less responsive. You typically have to push buttons for a certain time to get a response.
- In **Regular camera mode**, pressing the button just records one image in the **/camera** folder. If the button stays pushed, images are recorded continuously with a debouncing delay. It acts more or less as a burst mode.
- In **Motion sensor mode**, the DashBoy camera takes pictures automatically when it senses a motion. The threshold can be configured in the json configuration file.
- In **Slit-scan modes**, you can perform [**Slit-scan photography**](https://en.wikipedia.org/wiki/Slit-scan_photography). Image is recorded along the yellow line, one line per frame taken as fast as possible (or with a fixed delay inbetween, see config.json), until user presses the PUSH button again. In **slit scan mode inf.**, The central pixel vertical line is stacked until image reaches 4096 pixels (the loop automatically increments files if necessary, there is no limit in file quantity during one run, so the "infinite"). In **slit scan mode x128**, a 128 pixel width image is recorded by scanning slits along the whole sensor from left to rigth. Slit-scan mode disables the auto-exposure, the dithering and the HDR mode. Other recording modes (time lapse and motion sensor mode) are not affected. **A tripod is warmly recommended as you will loose display (on purpose) during recording.**

To activate **HDR mode** or **Dithering mode**, simply push the corresponding pushbuttons.

**HDR mode** (High Dynamical Range) take several images from -1EV to +1EV and make an average of them before recording. This increases the dynamic of the sensor. The list of exposures can be modified in the [config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json). **Keep the total length of the table at 8 entries !**

**Dithering mode** mimicks the dithering process of a Game Boy Camera with 4x4 derived Bayer matrices. Dithering matrices [generated online](https://herrzatacke.github.io/dither-pattern-gen/) can be copied in the **[config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json)** file. 

Finally, you can **lock exposure time from the current best one** in any mode by pressing the **LOCK_EX** push button. It displays a green screen border when it is on.

It is mandatory to format the SD card in FAT32 and it is better to use the maximum sector size possible to speed up writing and avoid stalling. The data transfer rate to the SD card is indeed the bottleneck of the device.

The device outputs 1x BMP images natively but the projects comes with a [bunch of tools to convert and upscale them and make videos from image series](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/SD).

# Using the DashBoy Camera "in a nutshell"

![DashBoy Camera in a nutshell](Docs%20and%20research/Image%20files/Commands_in_a_nutshell.png)

# Advanced user manual (requirement: playing with json file)

Additionally, you can address other cool features by entering them in the [config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json):
- **Game Boy Camera mode (default ON)** drives the DashBoy Camera exactly as the Game Boy Camera does, using the same register strategy and the same dithering matrices. Images made in this mode are pixel identical to images produces with a Game Boy Camera in the same lighting condition if dithering is activated. When set to 0, this triggers a simplier "recipe" with fixed registers and fixed dithering matrix.
- **Raw recording mode (default ON)** available in **time lapse mode** and **motion sensor mode**, records sensor data as a single file in the **/TL** or **/MS** folders respectively. It needs a separate tool for extracting an animation ([provided here](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/SD)). The 16 first bytes are keywords, image sizes (width and height in pixels) and registers, the following are raw data for one image in reading order, easy to recover with any other tool ! This mode allows recording at a solid 4 fps for long time with good light conditions. It is disabled in **Regular camera mode (point and shoot)**, command is just ignored. When disabled, images are just recorded in plain 1x BMP in separate folders.
- **Pretty border mode (default ON on first border)** generates 160x144 images with a fancy border like the Game Boy Camera. You can make your own with the tools provided to generate C-files. To roll between borders, just reboot the DashBoy while it accomodates for exposure after boot (between the two short red flashes).
- **Focus peaking (default OFF)** allows embossing the area with the best sharpness as focusing with such low definition on a tiny LCD screen can sometimes be tricky. Very practical if you [couple the device with a CS mount](https://www.thingiverse.com/thing:5024326), as the focusing plan can be hard to find with such low definition !
- **Night Mode (default OFF)** allows automatically downclocking the device in case the exposure registers reaches their theoretical maximal value (0xFFFF). This is usefull to do light painting from example, the sensor being natively not able to expose more than 1 second. Here there is no limit.
- **Fixed exposure mode (default OFF)** allows bypassing the autoexposure algorithm and fixing a value, useful for astrophotography where auto-exposure performs poorly. In this case you have to enter the exposure time or delay (between 0x0030 - 0.768 ms and 0xFFFF - 1.044 second) and the clock divider (a multiplier for the exposure time, stay at 1 if you do not know what to do.)
- **2D enhancement mode (default OFF)** enforces the 2D image enhancement processed by the sensor in Game Boy Camera mode & non dithering mode only, whatever the register used.

# Nerd user manual (requirement: recompiling the code)

You can also directly play with the **#define** options in [config.h](/Codes/128x160_Dashboy_Camera/config.h) and recompile the code to:
- deactivate the Leds (sneak mode);
- deactivate the SD card or the TFT screen to debug;
- activate the serial output and spit images as hexadecimal text to interface the camera with another software like Matlab, but it is rather slow. I used that initially for debugging without display, and I kept that mode for tricky bugs requiring access to the arrays for example;
- change screen driver from ST7735 128x128 pixels (default) to ST7789 256x256 pixels TFT display. Beware, you must also modify the config file (comment/uncomment stuff, everything is explained) for the TFT library, see the installtion guide;
- activate overclocking. I yet provide an overclocked build at the maximum possible frequency that do no crashes but it drains battery too fast to my own taste, despite being more responsive;
- deactivate the register display on screen or debug mode if it bothers you (why not after all, I though it was fun to keep it);
- activate the [debagame mode](https://tcrf.net/Proto:Game_Boy_Camera) that give access to advanced sensor calibration parameters. It displays on screen the voltage of masked pixels, the offset voltage, the register O and the register V and compare them to see how far the camera is from perfect calibration. If you do not see what it means, it's normal, this means you probably have a rewarding life and fulfilling social relationships.

# Some pictures made with the M64282FP sensor (old border)

![Dashboy Camera pictures](Docs%20and%20research/Image%20files/Examples.png)

# More pictures made with the M64282FP sensor (actual border)

![Dashboy Camera pictures](Docs%20and%20research/Image%20files/Examples_2.png)

Pictures were taken with the original plastic lens, CCTV lenses, CCTV fisheye with M12/CS adapters, and a [Tair-3s 300mm f/4.5 soviet telelens](http://nicolas.dupontbloch.free.fr/scope-tair-3s.htm) with a [M42 lens mount](https://www.thingiverse.com/thing:6077684) or sensor glued into a Zenit 12XP hand made shell adapter.

## Examples of slit-scan pictures and modes

![](/Docs%20and%20research/Image%20files/Slitscan.png)

The **slit-scan mode infinite** makes streaks whatever the scene. It gives interesting effects with long moving objects (trains) or natural scenes in the wind. It completely obliterates the background and only moving objects can be seen. It acts exactly as a [photo-finish](https://en.wikipedia.org/wiki/Photo_finish), in slower however due to sensor limitations. The **slit-scan mode x128** scans the whole scene line by line, the immobile backgroung is conserved. Only the moving objects of the scene will be deformed in a grotestque fashion. Using this mode with a Game Boy Camera sensor is an idea of [Andreas Hahn](https://github.com/HerrZatacke). Both mode loop infinitely as long as **PUSH** is not pressed.

## Some timelapses made with the M64282FP sensor

![Dashboy Camera pictures](Docs%20and%20research/Image%20files/Sun.gif)

![Dashboy Camera pictures](Docs%20and%20research/Image%20files/Moon.gif)

![Dashboy Camera pictures](Docs%20and%20research/Image%20files/Alps.gif)

## Example of dithered/non dithered image, M64282FP sensor

![Dashboy Camera pictures](Docs%20and%20research/Image%20files/Dither.gif)

## Examples of artifacts intrinsic to the M64282FP sensor

![Dashboy Camera pictures](Docs%20and%20research/Image%20files/Sensor_artifacts.gif)

## Examples of image/timelapse taken with a M64283FP sensor

![Dashboy Camera pictures](/Docs%20and%20research/Image%20files/M64283FP_example.png)

![Dashboy Camera pictures](/Docs%20and%20research/Image%20files/Stormy_weather.gif)

## Examples of artifacts intrinsic to the M64283FP sensor

![Dashboy Camera pictures](/Docs%20and%20research/Image%20files/Cloudy_weather.gif)

# Building the device !

## Required parts

**The total cost (Game Boy Camera not included), considering that you start from scratch, is about 35€ (PCB included). It has been designed to use ultra-cheap and ultra-common parts only.** Most of parts can be recycled from used electronics or Arduino related projects. If you're a bit into electronics, you probably yet have most of the parts somewhere on shelves. **It is fully through-hole on purpose so it necessitates nothing but very basic skills in soldering.**

So you will need: 
- [PCB and optional sub PCB](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/PCB/128x160_PCB%20and%20schematic) ordered at [JLCPCB](https://jlcpcb.com/). Just drop the Gerber .zip files on their site and order with default options.
- An [Arduino Pi Pico](https://fr.aliexpress.com/item/1005003928558306.html). **Be sure to select the regular/original green board with the official pinout and castellated holes ("Color: Pico Original").** And I know, the purple is [super fancy](Docs%20and%20research/Image%20files/Prototype.png), but the pinout is non standard and holes non castellated, do not choose this one.
- A [1.8 TFT SPI 128x160 display with SD card slot](https://fr.aliexpress.com/item/32964493983.html). Note that it is pixel perfect with the sensor, which is cool. Choose the red PCB one if any color choice. It must always come with a regular SD slot. Any similar one in another seller will do the job.
- Two [4 gates bidirectionnal level shifters](https://fr.aliexpress.com/item/1005004560297038.html). Any similar one in another seller will do the job.
- Some [9 pins JST/ZH1.5MM connector](https://fr.aliexpress.com/item/1005006028155508.html). Choose **vertical SMD**, ZH1.5MM, 9P. The PCB was made for the **vertical version only**, the horizontal will inverse polarity if you try to solder it anyway due to lack of clearance.
- Some [9 pins, double head, 10 cm, JST/ZH1.5MM cables](https://fr.aliexpress.com/item/1005004501408268.html). Beware, the Aliexpress models are overall crap, they tends to loose connection easily after several bending. In case of doubt or display bug, change the ribbon cable without hesitation.
- Some [male and female pin headers](https://fr.aliexpress.com/item/1005002577212594.html) if you want a removable display or removable level shifters (why not ?).
- A [DC-DC 5 volts regulator with buck boost converter](https://fr.aliexpress.com/item/32813355879.html). It is based on a ME2108 DC/DC converter chip. Due to small voltage ripples at output, it requires an extra beefy capacitor (see next parts).
- A [2xAA battery holder with cover](https://fr.aliexpress.com/item/1005004651271276.html). Do not take opened ones as they are generally crap and do not hold battery correctly.
- 2 [regular 5 mm LEDs](https://fr.aliexpress.com/item/32848810276.html) (red and green) and two [through hole resistors](https://fr.aliexpress.com/item/32866216363.html) of 250 to 1000 Ohms (low value = high brighness).
- A [16 volts through hole capacitor of 1000 microfarads](https://fr.aliexpress.com/item/1005003189675117.html).
- 6 [6x6 push buttons whatever their height](https://fr.aliexpress.com/item/1005003938244847.html).
- 2 [microswitches SS-12D00G](https://fr.aliexpress.com/item/1005003938856402.html) to cut the main power and the display backlight which draws more current (30 mA) than the Pi Pico (25 mA) itself, for saving battery in case of long timelapses for example.

The device is meant to be used with **AA NiMH batteries**. Lithium batteries are evil because they are unstable (I'm definitely dubious about long term storage of what will eventually end as spicy pillows and fire hazard), not recycled (less than 1% in the best IEA scenario) and non generic at all in terms of size in case of reuse. **The device does not allow to recharge these batteries** as NiMH requires very specific charging strategy. It can run with alkaline batteries too (even pretty discharged). You can of course alter the design to use it with Lithium batteries and embedded charger, it works perfectly too (my first design used a [DD05CVSA charge discharge](https://fr.aliexpress.com/item/33034500618.html) circuit). You can consult here the [sketch of the initial prototype](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/Prototype_pinout.png) to adapt a lithium battery charger. **Beware, this early prototype has pinout that differs significantly from the [current one](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/PCB/128x160_PCB%20and%20schematic/Main%20PCB/Schematic_Dashboy%20camera_NiMH_2023-05-14.pdf).**

## PCB and connection with the sensor

PCB are available in the [PCB](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/PCB/128x160_PCB%20and%20schematic) folder of the project. To connect the sensor to the board, use a female JST connector with bended pins in order to inverse the gender of the JST cable, or simply order the [breakout PCB](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/PCB/128x160_PCB%20and%20schematic/Sensor%20to%20PCB%20adapter%20board). Be carefull with the polarity: both faces of the cables must be the same (camera sensor cable side and extension cable side, see following images). This situation corresponds also to the INPUT<->NORMAL OUT of the breakout board if you order the extension cables given in **Required parts** section.

![Dashboy Camera connections](Docs%20and%20research/Image%20files/Ribbon_connection.png)

PCBs can be ordered at [JLCPCB](https://jlcpcb.com/) by simply uploading the gerber zip container to their site. JLCPCB has the advantage to be cheap and clean with the VAT for EU customers. Order HASL finish with 1.6 mm thickness for the two boards (default option). PCB designs and schematics can be edited with [EasyEDA Standard Edition](https://easyeda.com/fr).

## Building instructions

- Solder the Pi Pico first: **it must be mandatorily soldered on the castellated holes, without pin headers or spacer, directly on the PCB**
- Solder the lower parts next: LEDs (RED and GREEN), resistors (RRED and RGREEN), level shifters, push buttons, JST connector, microswitches (MAIN and DISPLAY), capacitor (CAP1) and DC-DC converter. Levels shifters and DC-DC converters can be soldered with male pin headers, there is enough clearance for them.
- Solder the display: **ensure to have enough clearance to slide the SD card out of the display above the Pi Pico**. Best is either to use female pin headers to secure the distance between display and main PCB or to solder the display with pins just showing out of the back PCB surface (pins are long enough).
- Trim the remaining pin connectors as short as possible from the back side of PCB.
- At this step, flash the Pi Pico and verify that the device boots and the display works. It should be stuck to an error screen with the red LED flashing, this is normal as the device needs SD card and a sensor to go further.
- **Before glueing anything to the back side, reflow any trimmed pin carefully.**
- Solder the JST connectors to the [Breakout PCB](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/PCB/128x160_PCB%20and%20schematic/Sensor%20to%20PCB%20adapter%20board) (INPUT and NORMAL OUT if you use a cable similar to the default sensor one linked in **Parts**) or just bend straight the pins of a female JST connector and plug it into the JST extension cable to inverse its gender.
- Trim the AA battery holder ON/OFF switch and place it in ON position, solder the battery terminals.
- Glue the Battery holder behind the pushbuttons (it will act as a hand grip) and the camera shell behind the display. Use double sided tape or dots of hotglue in order to allow reversing the mod and get back your precious camera shell if necessary.
- Connect the sensor to the extension cable or adapter board (beware of the polarity), place fresh AA batteries and enjoy your Dashboy Camera !
- If you want to use the time lapse or HDR features, use a tripod and hold the Dashboy with any [universal phone holder](https://fr.aliexpress.com/item/1005004208664097.html) by the battery holder.

## Image of the populated PCB as you will get it (PCB version 2.0, the current one)

![Dashboy populated PCB](Docs%20and%20research/Image%20files/PCB_version_2.0.png)

The GPIO22 pin, only digital pin left free on the Pi Pico, is exposed as well as a GND and 3.3V pins if you want to use them for other purpose like triggering something in/out of the device. It will of course require modifying and compiling the code.

# Some random informations for you Game Boy Camera nerd !

- The sensor has intrinsically [many visual artifacts](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor#example-of-image-artifacts) (horizontal and vertical lines, image echoes at certain exposures, etc.). None of them is specifically due to the Raspberry Pi Pico. They were all observed with a real Game Boy Camera in the same lighting conditions.
- The 16 bits exposure register C range should be limited to [0x0030-0xFFFF] according to Mistubishi, despite the sensor being technically able to cover [0x0001-0xFFFF]. This is advised in the sensor datasheet and the reason is that below 0x0030, the image artifacts becomes so intense (as black vertical bars) that they interfere with any auto-exposure algorithm based on pixel intensity. Technically, this creates exposure jitters and unpleasant behavior. The Game Boy Camera (and the Game Boy Camera mode here) has a range of exposure extended to 0x0010 by cancelling border enhancement but this however comes with a dark halo on the left side of image at very low exposure time (typically full sun in summer). Below 0x0010 and down to 0x001, the image covers less and less surface of the sensor with black pixels instead of data.
- The strategy used in Game Boy Camera mode and the auto-exposure algorithm have been reverse-engineered from real data logging on a regular Game Boy Camera. Source files can be found in this [other repository](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor/tree/main/Research%20on%20real%20Camera). Detailed explanations can be found [here](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor#concluding-remarks-and-some-dive-into-the-game-boy-camera-exposure-strategy). As I'm not stuck to 8-bits arithmetics in the Pi Pico, I took some freedom with my version of auto-exposure algorithm but it behaves overall the same. I however contributed to the [implementation of auto-exposure algorithm in Photo!](https://github.com/Raphael-Boichot/gb-photo/blob/5fab61e3a17b88dd25288fa2d2a604e5d07ccb91/src/state_camera.c#L998) by closely mimicking the real algorithm this time.
- Close up images of the sensor and the MAC-GBD decapped can be found [here](https://github.com/Raphael-Boichot/Game-Boy-chips-decapping-project). The sensor presents masked pixels which always return the reference voltage given by register V (raw voltage tuning) plus the saturation voltage. These pixels occupy the lines 123 to 128 of a 128x128 image and appears as a white (saturated) bar at the bottom. I do not see any way to get usefull information from these pixels.
- The Game Boy Camera has a [calibration procedure to adapt for each sensor](https://github.com/Raphael-Boichot/Inject-pictures-in-your-Game-Boy-Camera-saves#part-3-calibrating-the-sensor). It permanently modifies some sram values but the role of these data is not well documented. It anyway deduces a lookup table for register O (fine voltage tuning) to apply to have consistent images with different sensors at different gains and exposure times. I use here the calibration data of one particular camera I own, but measures between different cameras calibrated show very minor differences.
-  The Game Boy Camera uses registers to target a quite narrow voltage range (approx. 1.5-3.0V) as it has very limited post-processing capabilities (mainly handled by the MAC-GBD mapper which returns directly 2-bits dithered images data). In real conditions, the Game Boy Camera uses only the very upper half of the full voltage scale allowed by the MAC-GBD mapper. The [actual dithering matrices](https://github.com/HerrZatacke/dither-pattern-gen) used by the camera never code for value below 128 for this reason. The unused low voltage range serving as "voltage buffer" for bias (dark current) correction via register O. The DashBoy Camera emulates this feature in Game Boy Camera mode.
- The role of each registers is quite well understood now and [explained here](https://github.com/Raphael-Boichot/Play-with-the-Mitsubishi-M64283FP-sensor). The Mitsubishi [M64282FP datasheet](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/Mitsubishi%20Integrated%20Circuit%20M64282FP%20Image%20Sensor.pdf) is notorious for being crap so many informations were assessed by extrapolating things from the less sketchy (but also incomplete) [datasheet of the M64283FP](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/Mitsubishi%20Integrated%20Circuit%20M64283FP%20Image%20Sensor.pdf) (which is basically the same sensor with more features, but not better, see next). Undestanding the M64283FP datasheet still requires educated guess due to poor translation of the [original in Japanese](/Docs%20and%20research/Bibliography/Mitsubishi%20Integrated%20Circuit%20M64283FP%20Image%20Sensor%20(Original%20in%20Japanese).pdf) which is at least, this time, complete. This latter contains the exact dimensions of the sensor which is (voluntarily ?) obfuscated on the 82FP datasheet, comprising the focal plane position, that can be used for doing reliable lens modes with the 82FP (same package than the 83FP). The rushed datasheet for the M64282FP, only known version, may be due to the fact that the sensor being Japanese (even Nintendo of Japan) exclusive, English translation would have no real purpose. There must be a (lost) proprietary Japanese version, like the 83FP.

**Real dimensions of both the M64282FP and M64283FP from 83FP Japanese datasheet**
![](/Docs%20and%20research/Image%20files/Sensor_real_dimensions.png)
![](/Docs%20and%20research/Image%20files/Sensor_real_dimensions_2.png)
  
- The Game Boy camera rom injects only 5 bytes of registers while sensor itself requires 8 bytes. the 3 missing register bytes (P, M and X) are hard coded into the MAC-GBD and send to the camera anyway. Their main purpose is to inject custom convolution kernels. They can be bypassed anyway by using other registers offering the most usefull pre-calculated convolution kernels.
- The M64282FP sensor is known to have been massively produced for use in the Game Boy Camera. No known old new stock or other electronic device using it have been publically documented yet. On the other hand, the M64283FP, version with more features, was available for retail but no commercial device using it is known for sure at the moment.
- The mask marking on the M64283FP is **[A64283](https://drive.google.com/file/d/1iWyl2L_uUUIKef6FCX1JYUhLUcdR75x1/view)** (Mask revision A) while on the M64282FP it is **[B64282](https://drive.google.com/file/d/1t0iczgT00NVYwDEGJ-6-9WYSS4gzTDif/view)** (Mask revision B). Doing some science fiction, the MAC-GBD has pins with [undocumented purpose](https://github.com/Raphael-Boichot/Game-Boy-chips-decapping-project/blob/main/README.md#game-boy-camera-mac-gbd-mapper) that could have been used to push extra registers to a more complex prototype sensor (perhaps a version A of the 82FP ?).

**Mask marking on the surface of the M64282FP sensor**
![](/Docs%20and%20research/Image%20files/M64282FP_mask.png)

**Mask marking on the surface of the M64283FP sensor**
![](/Docs%20and%20research/Image%20files/M64283FP_mask.png)

- In scientific literature, the M64283FP has been used to produce experimental augmented vision systems, typically to treat tunnel vision. It was sometimes claimed in papers to be the sensor used with the Game Boy Camera which is factually wrong but which may also be a deliberate confusion maintained by Mitsubishi to obfuscate the M64282FP existence while it was produced for the camera (Mitsubishi never mention M64282FP Sensor in the literature). It anyways indicates that at some point, the M64283FP was available for retail, at least for labs (contrary to the M64282FP). It is also indicated that the M64283FP was quickly obsolete and dismissed in favor of full digital sensors with color and higher resolutions. The bottleneck of being stuck with an external ADC (even fast) processing pixel data in series is highlighted. The response of Mitsubishi was both to decrease the image resolution (M64285 series in FP or K version, 32x32 pixels, which also fits perfectly with neural networks of that era) or include the ADC to the sensor chip (M64287U and M64270G with internal 8-bit ADC).
- The M64283 sensor came in FP version (plastic case) and K version (ceramic package). Informations about the K version are very scarse (No clean image available online). It probably fills an industrial niche where hardened electronics and IR transparency are desired. On the other hand, no known M64282K version is reported anywhere on the internet.
- You can [directly drop a M64283FP on a Game Boy Camera sensor board](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/Sensor%20board%20with%20a%20M64283FP.png) as quite luxury replacement chip. [The image will just look softer](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/Image%20of%20a%20M64282FP%20taken%20with%20a%20M64283FP.png), to not say deceptive, as the table for E register (edge enhancement ratio) is not the same. Basically the 2D edge enhancement effect is cancelled if the [E register table used by the regular camera M64282FP sensor is injected into the M64283FP sensor](Docs%20and%20research/Image%20files/Register_E.png). The M64283FP sensor is also unstable during exposure changes, VOUT can goes to 0 when sensor is rapidely saturated in intense light and stay stuck in this state. Full (and tested) support is provided on the DashBoy Camera, patching these two issues (Sensor going crazy and table E register diffrence), in order to allow using the 83FP sensor smoothly and get pleasant images. Artifacts (vertical bars with 2D enhancement and ghost images) are very similar (but not identical) between the 82 and 83FP sensors.

**The M64283FP and M64282FP sensors are pinout compatible**
![](/Docs%20and%20research/Image%20files/Sensor%20comparison.png)

**M64283FP sensor directly drop on a PCB sensor board (and happy to take image like that)**
![](/Docs%20and%20research/Image%20files/Sensor%20board%20with%20a%20M64283FP.png)

**The register E used by the Game Boy Camera by default disables the edge enhancement of the M64283FP sensor**
![](/Docs%20and%20research/Image%20files/Register_E.png)

- The M64283FP has an automatic black level calibration ("clamping circuit") active by default by using the same P, M and X registers as a Game Boy Camera. My notes about that are in comments into the code. This leads to a perfect cancellation of offset voltage according to my own tests (so calibration is not needed, VOUT does not drift anymore with exposure time).
- One line of Japanese mobile phone ([the THZ43 Chiaro / LaPochee by Mitsubishi](https://time-space.kddi.com/ketaizukan/1999/10.html)) has allegedly used a 128x128 pixels artificial retina but it's unclear which (82 or 83FP or another one) without any tear down. The LaPochee device beeing horribly expensive, any tear down is unlikely to happen in the future. As the effective resolution of the LaPochee is 96x96 pixels, the M64283FP, which allows hardware cropping, is maybe embedded, but nothing is sure as software cropping is very easy to code. Ricoh also sold the [PCPICO](https://web.archive.org/web/20020925132513/http://pcpico.com/), a [1999 PC interface](https://ascii.jp/elem/000/000/315/315709/) which uses a 128x128 artificial retina [according to Mitsubishi](/Docs%20and%20research/Bibliography/Komuro%20(2011)%20Development%20and%20Commercialization%20of%20Artificial%20Retinal%20Chips.pdf) and promises an "Easy operation for the whole family from the day it is connected to the PC". Can you smell the 2000s marketing in that sentence ?
- The flash ADC of the MAC-GBD has a voltage full scale of 0<->3.3 Volts. Maximum voltage reached in normal use with a Game Boy Camera is about 3.05 volts. It can go until 3.18 volts during camera boot (may this be intentionnal or not). The ADC embedded in the MAC-GBD is protected from any overvoltage coming from the sensor by a low voltage Schottky Barrier Diode (more precisely a Panasonic MA784 or equivalent, about 0.18V forward voltage at 1 mA) which cathode is connected to the +3.0V of the board and anode connected to VOUT (analog image signal line). In the Dashboy Camera, I did not take this over-precautious measure as VOUT never goes over 3.05V (for the Pi Pico ADC going to 3.3V).
- The Game Boy Camera has two PCB revisions for its sensors. It is unclear wether it has an effect on images other than subjective or if the board revision is associated with a sensor revision. Veteran users of the camera admit that there is no clear separation between revisions in terms of image quality. The fact is that just cleaning the plastic lens generally cancels the subjective differences. The only solid observation is that early sensors used in Japanese cameras tends to present more vertical streaks artifacts (one vertical line over two is slighly darker).
- According to an [internal Mitsubishi source](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/Yerazunis%20(1999)%20An%20Inexpensive%2C%20All%20Solid-state%20Video%20and%20Data%20Recorder%20for%20Accident%20Reconstruction.pdf), the use of the M64282FP artificial retina for road safety dashcam application was assessed in 1999. They recommend using the [MAX153 flash ADC](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/MAX153%20-%201Msps%2C%20%C2%B5P-Compatible%2C8-Bit%20ADC%20with%201%C2%B5A%20Power-Down.pdf) to convert analog signal fast enough for a live (5 fps !) rendering and recording of images. The MAC-GBD, mapper of the Game Boy Camera, [embeds a flash ADC](https://github.com/Raphael-Boichot/Game-Boy-chips-decapping-project#game-boy-camera-mac-gbd-mapper) too on its chip. The probability is thin but not zero that the MAC-GBD flash ADC would be simply a MAX153 adapted for this custom mapper. A copy of this experiment [is proposed here](https://github.com/Raphael-Boichot/Game-Boy-camera-sniffer). And yes it works !
- Oh, and the "dancing man" in the Game Boy Camera credits is [Kentaro Nishimura](https://nintendo.fandom.com/wiki/Kentaro_Nishimura), not Shigeru Miyamoto. He's also probably the unknown face in the [Debagame Tester: Second Impact (デバガメテスター SECOND IMPACT)](https://tcrf.net/Proto:Game_Boy_Camera), same jacket, same glasses, same haircut, same kind of humor.

**Kentaro Nishimura aka "the dancing man" of Game Boy Camera**
![Game Boy Camera dancing man](/Docs%20and%20research/Image%20files/JP_Dancing_man.jpg)

**The unknown face hidden in the Debagame Tester: Second Impact, probably Kentaro Nishimura too**
- ![Debagame tester game boy camera](/Docs%20and%20research/Image%20files/DebagameTesterSI-Pictures.png)

# My feedback about some Game Boy Camera lens modes I've tried with the GameBoy/Dashboy Camera
  
- First, the original plastic lens used in 8-bits mode gives a quite eery image with vignetting. In a word: it is crap. But it also has its own charm. Everything looks "ashy" with it. And the original camera eye ball allows making selfies contrary to lens modes ! 
- If you use a lens mod with the DashBoy camera, the lens aperture will define how dirty will look your sensor. The wider the opening, the less artifacts on the image. These speckles, due to dust or sensor acrylic surface roughness or scratches, can barely be seens with a regular Game Boy Camera due to dithering but becomes very obvious in 8 bits mode where the image is much smoother, in particular in timelapse mode. So do not worry, the DashBoy Camera does not wear your sensor, just open your diaphragm (if any) !
- The crop factor between a full frame sensor and the camera sensor is about 10. This means that even the marvelous 16 mm Zenitar from the photographic film era will act as a boring 160 mm telephoto lens, but on the other hand, a Tair 3S 300 mm lens will becomes a sexy 3000 mm ready for astrophotography. For the same reasons, CCTV lenses will began to act as "wide angle" with the Game Boy Camera below 2.8 mm.
- Most camera lens modes are based on 3D printed stuff with very rough tolerances, based themselves on schematics found by reverse engineering the actual sensor position in its acrylic enclosure (with its own tolerances too). So there are two typical cases: either the focale plane is behind the surface sensor at infinity, which can be fixed with lens focusing (if any), either the focal plane is ahead at infinity, with means that you can NEVER get a focused image. The reason is that the lens is too far from the sensor. The only way to fix that is to shave the front plane of the 3D parts and make trials BEFORE gluing anything (like a screw mount) on it. Note to tinkerers: better have a sensor a bit too close that a bit too far from the lens. Too close can be easily fixed with washers for example (in addition to focusing of course).
- This focusing plane "intolerance to tolerances" is particularly obvious with fixed focal wide angle CCTV lenses (like a 1.8 mm). So if a lens mount works with them, it will work with anything else !

# Things that were dismissed during dev because any project must be finished one day.

- Porting back the whole code to ESP32, the Pi Pico version beeing itself ported from the [prototype ESP32 version](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor/tree/main/ESP32_version_beta). I've tried, then gave up, too many memory management issues.
- Direct recording in PNG format with the [PNGenc library](https://github.com/bitbank2/PNGenc), instead of BMP. I managed to embed this library into the [NeoGB printer](https://github.com/zenaro147/NeoGB-Printer), but no matter how I tune the memory management in the DashBoy Camera project, I sometimes ran out of memory despite the fact that I still have "plenty". So I have a version 99% working in the drawers but not 100%... And BMP format just works fine in fact, I very like this encoding format, it's easy to handle.
- Adding a menuing system with a single joystick was imagined, but it requires basically starting the code from scratch. In the same mood, a full touchscreen interface would be cool and probably easier to code.
- Making a full SMD device small and pretty (with sensor included on the PCB and a printed shell) to make a tiny digital camera. Too much work knowing that the current design perfectly fills my needs.
- Adding an USB support with [tinyusb](https://github.com/earlephilhower/Adafruit_TinyUSB_Arduino/tree/master) to turn the device into a webcam by using the second core. I did not manage to make any Arduino IDE compatible library working with Windows 10 USB camera support and gave up. Maybe in the future  if I give another try. Honestly not a priority, I would have no use of this feature.

# Some dev notes

- This device is "8-bit" here on purpose but as the sensor outputs analog pixel data, it can become whatever number of bits the ADC you use to convert the signal is able to spit. With the Pi Pico, the ADC is 12 bits (so I've cut the 4 LSB). It is so theoretically possible to store sensor signal in 12 bits per pixels and perform some kind of post-treatment. It is of course totally pointless seing the intrinsic signal to noise ratio of the sensor.
- The extra registers used by the M64283FP sensor (allowing hardware cropping and projections) are not implemented here as they were only intended to fasten the post-processing step in the mid 90s. Here the calculation power available is infinite compared to these times. It's anyway intersting to see how they work with a very slow device so you can jump to [this project to play with them on Arduino](https://github.com/Raphael-Boichot/Play-with-the-Mitsubishi-M64283FP-sensor).

# Troubleshooting

- The screen stays white after flashing: error of TFT library linking, follow closely the installation guide with the Bodmer library and flash again;
- The image colors on screen are inacurrate and the default parameters are lost: json config file corruption, the device runs on internal miminal configuration, copy a fresh config.json from repo to the SD card and reboot. Cables between sensor en main board may be loose too;
- The image looks too dark or too bright on screen despite the auto-exposure working, the image jitters: loose connection with sensor, re-seat the 9 pins connector on both sides.
- the screen shuts down but the auto-exposure continues to work: empty batteries;
- the device works but very slowly: SD card corruption, the device struggles to access it so the slowdown.

# Kind warning

The code and current design come as it. If you're not happy with the current hardware, the PCB EasyEDA design or the Arduino IDE, create your own, the licence allows it ! Push request with tested and working improvements are of course still welcomed.

# Acknowledgments

- [Andreas Hahn](https://github.com/HerrZatacke) for the fruitful technical dicussions, the [dithering pattern generator](https://herrzatacke.github.io/dither-pattern-gen/) the [Powershell scripts](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/SD), the [colorscale generator](https://github.com/HerrZatacke/gradient-values) and the [M64283FP/82FP-compatible PCB](https://github.com/HerrZatacke/M64283FP-Camera-PCB). He provided many ideas, tools, concepts and design feedbacks that leads to this quite polished version, as well as a project for an ultra-small device with a 256x256 pixels display using a [ST7789 driver yet included in the code](Docs%20and%20research/Image%20files/Support_for_ST7789_driver.png) that I hope will one day become a reality.
- Razole for providing me some M64283FPs "under the counter" in exchange of the #000001 exemplary of the current device. If you read this one day, I've fully implemented the advanced sensor function in this [other project](https://github.com/Raphael-Boichot/Play-with-the-Mitsubishi-M64283FP-sensor/tree/main).
- [Rafael Zenaro](https://github.com/zenaro147) because I stole chunks of code and ideas from the [NeoGB Printer project](https://github.com/zenaro147/NeoGB-Printer). Coding myself for the NeoGB printer on the crappy ESP32 gave me (nightmares) enough confidence to dare starting this quite convoluted project. Great thanks my friend, whatever you do now !
- Drago for giving me some [M42 lens mount](https://www.thingiverse.com/thing:6077684) in exchange of a NeoGB printer, allowing me to mount the puny camera sensor on my ENORMOUS Tair-3s 300mm f/4.5 soviet telelens (so the Moon).
- [Michael Shimniok](https://github.com/shimniok/avr-gameboy-cam) and/or [Laurent Saint-Marcel](https://github.com/BackupGGCode/avr-gameboy-cam) for the Arduino code I started from (initial [source](http://sophiateam.undrgnd.free.fr/microcontroller/camera/) here). I'm not 100% sure of who is the initial author so I apologize for any error in citation.

# Developpement steps

  ![Dashboy Camera prototype](Docs%20and%20research/Image%20files/Developpment_steps.gif)

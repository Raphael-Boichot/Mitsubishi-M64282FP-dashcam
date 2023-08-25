# The DASHBOY CAMERA

## A digital camera made from the Mitsubishi M64282FP artificial retina and a Raspberry Pi Pico. 
The project fully emulates the sensor management strategy of the Game Boy Camera while completely bypassing the MAC-GBD mapper, which means you can record pixel perfect images without the limitation of storage space or the need for a Game Boy Printer emulator. The device can output dithered 2 bits per pixel images (like the Game Boy Camera) as well as smooth 8 bits per pixel images that take advantage of the full sensor bit-depth. It features time lapse recording, live recording, motion detection, High Dynamic Range, multi-exposure, night mode, Peak Focusing, fancy borders and even more. It's a fully open project so fork it and play with it !

**The code was developped for Raspberry Pi Pico** with the Earle F. Philhower RP2040 core. TFT display is driven with the Bodmer TFT_eSPI library. The code originates from an [Arduino version](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor) that requires a PC. This project was preceded by a [regrettable attempt](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor#attempt-to-make-a-mobile-device-with-an-esp32) to do the same with an ESP32 (to be honest, I did not know what I was doing at that time but the dev on ESP32 was a nightmare in comparison to Pi Pico). You may also check this similar project using a [STM32](https://github.com/mupfdev/CMOS-Holga) and a Holga camera.

If you manage to touch the [unobtainium M64283FP](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/Mitsubishi%20Integrated%20Circuit%20M64283FP%20Image%20Sensor.pdf), "enhanced" version of the [M64282FP](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/Mitsubishi%20Integrated%20Circuit%20M64282FP%20Image%20Sensor.pdf), it's also compatible with the DashBoy Camera (and the Game Boy Camera in general). I was lucky enough to obtain a few of them, and was able to test them in real-life conditions. This sensor produces very smooth (to not say blurry) images due to an increased IR sensitivity and some subtle register table change.

Want to discuss with the author or other Game Boy Camera nerds ? Join the [Game Boy Camera Club !](https://discord.gg/TDrcFKxAMU)

# Picture of the device (example of PCB V1.0)

![Dashboy Camera V1.0](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/PCB_version_1.0.png)

# Multi-systems installation guide

- **Just drop the [compiled uf2 file](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/Build) to the board in mass storage media mode (Connect the Pico to USB port with the BOOTSEL button pushed and release).**

Or if you [like pain](https://www.youtube.com/watch?v=IaUbdIDowJQ):

- Install the last [Arduino IDE](https://www.arduino.cc/en/software)
- Install the [Earle F. Philhower Raspberry Pi Pico Arduino core for Arduino IDE](https://github.com/earlephilhower/arduino-pico) via the Arduino Board manager (see [installation guide](https://github.com/earlephilhower/arduino-pico#installing-via-arduino-boards-manager)).
- Install the Benoit Blanchon [Arduino JSON library](https://github.com/bblanchon/ArduinoJson) via the Arduino library manager.
- Import or install the Bodmer [TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI) via the Arduino library manager.
- Locate the TFT_eSPI library: **\Arduino\libraries\TFT_eSPI** folder in your Arduino libraries
    
- copy the [configuration file](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/Codes/128x160%20THT%20version/128x160_Dashboy_Camera/128x160_TFT%20Setup) for the TFT display in this folder.
- edit the User_Setup_Select.h and modify line 29:

    **#include <Mitsubishi_M64282FP_dashcam_TFT_eSPI_setup.h> // Default setup is root library folder**
    
- Compile your code **with overclock set at 250 MHz** and flash the .uf2 to your board (Arduino IDE does that automatically once you've pressed the BOOTSEL button once).

# Basic user manual

The **Dashboy Camera** uses a [configuration file](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json) that must be placed at the root of SD card which has priority on the internal default parameters. The device cannot boot without a working sensor and a working SD card. Status of sensor, SD card and configuration file are indicated on the splashscreen.

- Once the device is booted, it adapts the sensor exposure for 2-3 seconds then runs in **Display mode**. Display mode does nothing else than showing on display what the Mitsubishi sensor sees at the maximum allowed refresh rate. The green LED indicates the exposure time. It can go from approx 1 ms to 1 second depending on lighting conditions. The red LED indicates access to the SD card for recording (or error at boot). 
- There are three main modes: **Regular Camera mode** to take picture one by one like any camera, **Motion sensor mode** that triggers pictures according to image change and **Timelapse mode** to automatically take pictures with a delay inbetween. Shifting from one mode to the other is made by pushing the **TIMELAPSE** button. It rolls over a list of 6 exposure delays for time lapse and the two other modes. In the json configuration file, delays are entered in ms, a value of -1 indicates a return to **Regular camera mode**, a value of -2 indicates a shift to **Motion sensor mode**. **Keep the total length of the table at 8 entries !** 
- In **Timelapse mode**, each recording session uses a different folder to store images. Timelapse mode has virtually no ending, it can go as long as the SD card is not full. The device enters a sleeping loop every 1/10 of the timelapse delay and wake up only briefly to accomodate exposure. The happy consequence is that electrical consumption falls to almost zero. The drawback is that the interface is less responsive. You typically have to push buttons for a certain time to get a response.
- In **Regular camera mode**, pressing the button just records one image in the **/camera** folder. If the button stays pushed, images are recorded continuously with a debouncing delay of 150 ms. It acts more or less as a burst mode.
- In **Motion sensor mode**, the DashBoy camera takes pictures automatically when it senses a motion. The threshold can be configured in the json configuration file.

To activate **HDR mode** or **Dithering mode**, simply push the corresponding pushbuttons.

**HDR mode** take several images from -1EV to +1EV and make an average of them before recording. This increases the dynamic of the sensor. The list of exposures can be modified in the [config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json). **Keep the total length of the table at 8 entries !**

**Dithering mode** mimicks the dithering process of a Game Boy Camera with 4x4 derived Bayer matrices. Dithering matrices [generated online](https://herrzatacke.github.io/dither-pattern-gen/) can be copied in the **[config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json)** file. 

Finally, you can **lock exposure time from the current best one** in any mode by pressing the **LOCK_EX** push button. It displays a green screen border when it is on.

It is mandatory to format the SD card in FAT32 and it is better to use the maximum sector size possible to speed up writing and avoid stalling. The access to the SD card is indeed the bottleneck in Recording mode.

The device outputs 1x BMP images natively but the projects comes with a [bunch of tools to convert and upscale them and make videos from image series](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/SD).

# Advanced user manual

Additionally, you can address other cool features by entering them in the [config.json](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/SD/config.json):
- **Raw recording mode** available in **time lapse mode** and **motion sensor mode**, records sensor data as a single file in the **/TL** or **/MS** folders respectively. It needs a separate tool for extracting an animation ([provided here](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/SD)). The 16 first bytes are keywords, image sizes (width and height in pixels) and registers, the following are raw data for one image in reading order, easy to recover with any other tool ! This mode allows recording at a solid 4 fps for long time with good light conditions. It is disabled in **Regular camera mode** (command is just ignored).
- **Night Mode** allows automatically downclocking the device in case the exposure registers reaches their theoretical maximal value (0xFFFF). This is usefull to do light painting from example, the sensor being natively not able to expose more than 1 second. Here there is no limit.
- **Fixed exposure mode** allows bypassing the autoexposure algorithm and fixing a value, useful for astrophotography where auto-exposure performs poorly. In this case you have to enter the exposure time or delay (between 0x0030 - 0.768 ms and 0xFFFF - 1.044 second) and the clock divider (a multiplier for the exposure time, stay at 1 if you do not know what to do.)
- **Pretty border mode** generates 160x144 images with a fancy border like the Game Boy Camera. You can make your own with the tools provided to generate C-files. To roll between borders, just reboot the DashBoy while it accomodates for exposure after boot.
- **2D enhancement mode** enforces the 2D image enhancement processed by the sensor in Game Boy Camera mode & non dithering mode only.
- **Game Boy Camera mode** drives the DashBoy Camera exactly as the Game Boy Camera does, using the same registers and the same dithering matrices in default constrast. Images made in this mode are pixel identical to images produces with a Game Boy Camera in the same lightning condition if dithering is activated. When set to 0, this triggers a simplier "recipe" with fixed registers and fixed dithering matrix.
- **Focus peaking** allows embossing the area with the best exposure as focusing with such low definition can sometimes be tricky.

# Some pictures made with the device

![Dashboy Camera pictures](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/Examples.png)

Pictures were taken with the original plastic lens, [CCTV lenses](https://fr.aliexpress.com/item/1005001617519795.html) with [CS mount mod](https://www.thingiverse.com/thing:5024326), [CCTV fisheye with M12/CS adapters](https://fr.aliexpress.com/item/1005005496283838.html) and a [Tair-3s 300mm f/4.5 soviet telelens](http://nicolas.dupontbloch.free.fr/scope-tair-3s.htm), sensor glued into a Zenit 12XP shell.

# Building the device !

## Required parts

**The total cost (Game Boy Camera not included), considering that you start from scratch, is about 35€ (PCB included). It has been designed to use ultra-cheap and ultra-common parts only.** Most of parts can be recycled from used electronics or Arduino related projects. If you're a bit into electronics, you probably yet have most of the parts somewhere on shelves. **It is fully through-hole so it necessitates nothing but very basic skills in soldering.**

So you will need: 
- [PCB and optional sub PCB](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/PCB/128x160_PCB%20and%20schematic) ordered at [JLCPCB](https://jlcpcb.com/). Just drop the Gerber .zip files on their site and order with default options.
- An [Arduino Pi Pico](https://fr.aliexpress.com/item/1005003928558306.html). **Be sure to select the regular/original green board with the official pinout and castellated holes ("Color: Pico Original").**
- A [1.8 TFT SPI 128x160 display](https://fr.aliexpress.com/item/1005004536839797.html). Note that it is pixel perfect with the sensor, which is cool.
- Two [4 gates bidirectionnal level shifters](https://fr.aliexpress.com/item/1005004560297038.html).
- Some [9 pins JST ZH1.5MM connector](https://fr.aliexpress.com/item/32920487056.html). If it comes with straight pins, you can bend them easily.
- Some [9 pins, double head, 10 cm, JST ZH1.5MM cables](https://fr.aliexpress.com/item/1005004501408268.html).
- Some [male and female pin headers](https://fr.aliexpress.com/item/1005002577212594.html).
- A [DC-DC 5 volts regulator](https://fr.aliexpress.com/item/32813355879.html). It is based on a ME2108 DC/DC converter chip. Due to small voltage ripples, it requires an extra capacitor (see next parts).
- A [2xAA battery holder with cover](https://fr.aliexpress.com/item/1005004651271276.html). Do not take opened ones as they are generally crap and do not hold battery correctly.
- 2 [regular 5 mm LEDs](https://fr.aliexpress.com/item/32848810276.html) (red and green) and two [through hole resistors](https://fr.aliexpress.com/item/32866216363.html) of 250 to 1000 Ohms (low value = high brighness).
- A [16 volts through hole capacitor of 1000 microfarads](https://fr.aliexpress.com/item/1005003189675117.html).
- 6 [6x6 push buttons whatever their height](https://fr.aliexpress.com/item/1005003938244847.html).
- 2 [microswitches SS-12D00G](https://fr.aliexpress.com/item/1005003938856402.html) to cut the main power and the display backlight which draws more current (30 mA) than the Pi Pico (25 mA) itself, for saving battery in case of long timelapses for example.

The device is meant to be used with **AA NiMH batteries**. Lithium batteries are evil because they are unstable, not well recycled and non generic at all in terms of size in case of reuse. **The device does not allow to recharge these batteries** as NiMH requires very specific charging strategy. It can run with alkaline batteries too (even pretty discharged). You can of course alter the design to use it with Lithium batteries and embedded charger, it works perfectly too (my first design used a [DD05CVSA charge discharge](https://fr.aliexpress.com/item/33034500618.html) circuit). You can consult here the [sketch of the initial prototype](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/Prototype_pinout.png) to adapt a lithium battery charger. **Beware, this early prototype has pinout that differs significantly from the [current one](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/PCB/128x160_PCB%20and%20schematic/Main%20PCB/Schematic_Dashboy%20camera_NiMH_2023-05-14.pdf).**

## PCB and connection with the sensor

PCB are available in the [PCB](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/PCB/128x160_PCB%20and%20schematic) folder of the project. To connect the sensor to the board, use a female JST connector with bended pins in order to inverse the gender of the JST cable, or simply order the [breakout PCB](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/PCB/128x160_PCB%20and%20schematic/Sensor%20to%20PCB%20adapter%20board). Be carefull with the polarity: both faces of the cables must be the same (camera sensor cable side and extension cable side, see following images). This situation corresponds also to the INPUT<->NORMAL OUT of the breakout board if you order the extension cables given in **Required parts** section.

![Dashboy Camera connections](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/Ribbon_connection.png)

PCBs can be ordered at [JLCPCB](https://jlcpcb.com/) by simply uploading the gerber zip container to their site. JLCPCB has the advantage to be cheap and clean with the VAT for EU customers. Order HASL finish with 1.6 mm thickness for the two boards (default option). PCB designs and schematics can be edited with [EasyDA](https://easyeda.com/fr).

## Building instructions

- Solder the Pi Pico first: **it must be mandatorily soldered on the castellated holes, without pin headers or spacer, directly on the PCB**
- Solder the lower parts next: LEDs (RED and GREEN), resistors (RRED and RGREEN), level shifters, push buttons, JST connector, microswitches (MAIN and DISPLAY), capacitor (CAP1) and DC-DC converter. Levels shifters and DC-DC converters can be soldered with male pin headers, there is enough clearance for them.
- Solder the display: **ensure to have enough clearance to slide the SD card out of the display above the Pi Pico**. Best is either to use female pin headers to secure the distance between display and main PCB or to solder the display with pins just showing out of the back PCB surface (pins are long enough).
- Trim the remaining pin connectors as short as possible from the back side of PCB.
- At this step, flash the Pi Pico and verify that the device boots and the display works. It should be stuck to an error screen with the red LED flashing, this is normal as the device needs SD card and a sensor to go further.
- Solder the JST connectors to the [tiny adapter board](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/128x160%20THT%20version/128x160_PCB%20and%20schematic/Sensor%20to%20PCB%20adapter%20board/Tiny%20adapter%20board.png) (INPUT and NORMAL OUT if you use a cable similar to the default sensor one linked in **Parts**) or just bend straight the pins of a female JST connector and plug it into the JST extension cable to inverse its gender.
- Trim the AA battery holder ON/OFF switch and place it in ON position, solder the battery terminals.
- Glue the Battery holder behind the pushbuttons (it will act as a hand grip) and the camera shell behind the display. Use double sided tape or dots of hotglue in order to allow reversing the mod and get back your precious camera shell if necessary.
- Connect the sensor to the extension cable or adapter board (beware of the polarity), place fresh AA batteries and enjoy your Dashboy Camera !
- If you want to use the time lapse or HDR features, use a tripod and hold the Dashboy with any [universal phone holder](https://fr.aliexpress.com/item/1005004208664097.html) by the battery holder.

## Image of the populated PCB as you will get it

![Dashboy populated PCB](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/PCB_version_2.0.png)

# Some random informations for you Game Boy Camera nerd !

- The sensor has intrinsically [many visual artifacts](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor#example-of-image-artifacts) (horizontal and vertical lines, image echoes at certain exposures, etc.). None of them is specifically due to the Raspberry Pi Pico. They were all observed with a real Game Boy Camera in the same lighting conditions.
- The 16 bits exposure register range is normally limited to [0x0030-0xFFFF] despite the camera being technically able to cover [0x0001-0xFFFF]. This is advised in the sensor datasheet and the reason is that below 0x0030, the image artifacts becomes so intense that they interfere with any auto-exposure algorithm based on pixel intensity. Technically, this creates exposure jitters and unpleasant behavior. The Game Boy Camera mode have a low range of exposure extended to 0x0010 by playing on other registers at the same time but this however comes with a dark halo on the left side of image at very low exposure time (typically full sun in summer).
- The strategy used in Game Boy Camera mode and the auto-exposure algorithm have been sourced from real data logging on a regular Game Boy Camera. Source files can be found in this [other repository](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor/tree/main/Research%20on%20real%20Camera). Detailed explanations can be found [here](https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor#concluding-remarks-and-some-dive-into-the-game-boy-camera-exposure-strategy).
- Close up images of the sensor and the MAC-GBD decapped can be found [here](https://github.com/Raphael-Boichot/Game-Boy-chips-decapping-project). The sensor presents masked pixels which always return the reference voltage given by register V plus or minus a small bias (dark current) which shifts with exposure time. Register O is intended to manually cancel this bias. Black pixels occupy the lines 123 to 128 of a 128x128 image. Correcting the bias is not mandatory to get clean images as long as digital post-processing is available. In this regard, the Game Boy Camera uses registers to target a quite narrow voltage range as it has very limited post-processing capabilities (mainly handled by the MAC-GBD mapper which returns directly 2-bits dithered images).
- The Game Boy Camera has a [calibration procedure to adapt for each sensor](https://github.com/Raphael-Boichot/Inject-pictures-in-your-Game-Boy-Camera-saves#part-3-calibrating-the-sensor) which effect is not clearly understood at the moment (it changes some sram values but the role of these data is not documented yet). It probably deduces a bias to apply in order to have consistent images with different sensors (Register O may be used for this task). Other say, the results you will get with the DashBoy Camera project may vary with the sensor used as I use a fixed calibration on my side.
- The role of each registers is quite well understood now and [explained here](https://github.com/untoxa/gb-photo#effect-of-the-main-adressable-parameters). The Mitsubishi [M64282FP datasheet](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/Mitsubishi%20Integrated%20Circuit%20M64282FP%20Image%20Sensor.pdf) is notorious for being crap so many informations were assessed by extrapolating things from the less sketchy [datasheet of the M64283FP](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/Mitsubishi%20Integrated%20Circuit%20M64283FP%20Image%20Sensor.pdf) (which is basically the same sensor with more features). Undestanding the M64283FP datasheet still requires educated guess due to translation approximations and internal inconsistencies.
- The Game Boy camera uses only 5 bytes of registers while sensor itself requires 8 bytes. the 3 missing register bytes (P, M and X) are hard coded into the MAC-GBD and send to the camera anyway. Their main purpose is to inject custom convolution kernels. They can be bypassed by using some other registers offering most usefull pre-calculated convolution kernels.
- The M64282FP sensor is known to have been massively produced for use in the Game Boy Camera. No known old new stock or other electronic device using it have been publically documented yet. On the other hand, the M64283FP, more advanced version, was produced but no device using it is known for sure at the moment. Old new stocks sometimes appears on the internet. The M64283FP is [backward compatible](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/Sensor%20comparison.png) with the M64282FP. Registers to drive them are nearly identical. One line of Japanese mobile phone ([the THZ43 Chiaro / LaPochee by Mitsubishi](https://time-space.kddi.com/ketaizukan/1999/10.html)) has used one of these sensors but it's unclear which without any tear down (the LaPochee device beeing horribly expensive, any tear down is unlikely to happen in the future). As the resolution of the LaPochee is 96x96 pixels, the M64283FP, which allows hardware cropping, is maybe embedded.
- You can [directly drop a M64283FP on a Game Boy Camera sensor board](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/Sensor%20board%20with%20a%20M64283FP.png) as quite luxury replacement chip. [The image will just look softer](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/Image%20of%20a%20M64282FP%20taken%20with%20a%20M64283FP.png) as the table for E register (edge enhancement ratio) is not the same. Basically the 2D edge enhancement effect is cancelled with the regular camera registers and this sensor. Full (and tested) support is provided on the DashBoy Camera.
- The flash ADC of the MAC-GBD has a voltage full scale of 0<->3.3 Volts (even probably 3.36V, seing the effect of dithering matrices on Rapsberry Pi Pico raw data). Maximum voltage reached in normal use is about 3.05 volts. It can go until 3.18 volts during camera boot (may this be intentionnal or not).
- The Game Boy Camera has two PCB revisions for its sensors. It is unclear wether it has an effect on images other than subjective or if the board revision is associated with a sensor revision. Veteran users of the camera admit that there is no clear separation between revisions in terms of image quality. The fact is that just cleaning the plastic lens generally cancels the subjective differences.
- If you use a lens mod with the DashBoy camera, the lens aperture will define how dirty will look your sensor. The wider the opening, the less artifacts on the image. These speckles due to dust or surface roughness can barely be seens with a regular Game Boy Camera due to dithering but becomes very obvious in 8 bits mode where the image is much smoother. So the DashBoy does not wear the sensor, it just reveals its imperfections.
- According to an [internal Mitsubishi source](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/Yerazunis%20(1999)%20An%20Inexpensive%2C%20All%20Solid-state%20Video%20and%20Data%20Recorder%20for%20Accident%20Reconstruction.pdf), the use of the M64282FP artificial retina for road safety dashcam application was assessed in 1999. They recommend using the [MAX153 flash ADC](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Bibliography/MAX153%20-%201Msps%2C%20%C2%B5P-Compatible%2C8-Bit%20ADC%20with%201%C2%B5A%20Power-Down.pdf) to convert analog signal fast enough for a live (5 fps !) rendering and recording of images. The MAC-GBD, mapper of the Game Boy Camera, [embeds a flash ADC](https://github.com/Raphael-Boichot/Game-Boy-chips-decapping-project#game-boy-camera-mac-gbd-mapper) too on its chip. The probability is thin but not zero that the MAC-GBD flash ADC would be simply a MAX153 adapted for this custom mapper. A copy of this experiment [is proposed here](https://github.com/Raphael-Boichot/Game-Boy-camera-sniffer). And yes it works !

# Kind warning

The code and current design come as it. If you're not happy with the current hardware, the PCB EasyDA design or the Arduino IDE, create your own, the licence allows it ! Push request with tested and working improvements are of course still welcomed.

# Acknowledgments

- [Game Boy Camera Club](https://disboard.org/fr/server/568464159050694666) on Discord for the hype and help on all new projects. You can discuss with the authors here !
- [Rafael Zenaro](https://github.com/zenaro147) because I stole chunks of code from the [NeoGB Printer project](https://github.com/zenaro147/NeoGB-Printer).
- [Andreas Hahn](https://github.com/HerrZatacke) for the fruitful technical dicussions, the [dithering pattern generator](https://herrzatacke.github.io/dither-pattern-gen/) the [Powershell scripts](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/tree/main/SD) and the [colorscale generator](https://github.com/HerrZatacke/gradient-values).
- [Michael Shimniok](https://github.com/shimniok/avr-gameboy-cam) and [Laurent Saint-Marcel](https://github.com/BackupGGCode/avr-gameboy-cam) for the Arduino code I started from (initial [source](http://sophiateam.undrgnd.free.fr/microcontroller/camera/) here). I'm not 100% sure of who is the initial author so I apologize for any error in citation.

# Developpement steps

  ![Dashboy Camera prototype](https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/Developpment_steps.gif)

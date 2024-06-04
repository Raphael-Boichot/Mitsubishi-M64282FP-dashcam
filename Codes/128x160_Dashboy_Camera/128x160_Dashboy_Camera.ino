//By Raphaël BOICHOT, made around 2023-01-27. Beware, I'm not a developper at all so it won't be a pretty code !
//Version 2.0 (1.0 was on ESP32 https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor/tree/main/ESP32_version_beta)
//Code is now too huge to be ported back on ESP32 due to its puny memory (theoretically 520 kB, in fact 160 kB but in real less than 110 kB without using sorcery)
//I've minimized the use of local variables, pointers and structures to have a constant clear view of what happens in memory because I'm dumb.
//The code is written to go FAST, so the use of numerous lookup tables and precalculated arrays always present into ram.
//Started from a code of Laurent Saint-Marcel (lstmarcel@yahoo.fr) written in 2005/07/05 but I'm not 100% sure of the very first author, anyway, only few lines of the starting code are still there
//I also stole some code for Rafael Zenaro's NeoGB printer: https://github.com/zenaro147/NeoGB-Printer
//Version for Arduino here (requires a computer): https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor
//Made to be compiled on the Arduino IDE, using these libraries:
//https://github.com/earlephilhower/arduino-pico core for pi-pico
//https://github.com/Bodmer/TFT_eSPI library for TFT display
//https://arduinojson.org/ for config.json file support

//some general calls to deal with the RP2040 core for Arduino and the json stuff
#include "ArduinoJson.h"   //self explanatory
#include "pico/stdlib.h"   //stuff from pico sdk
#include "hardware/adc.h"  //the GPIO commands are here
#include "config.h"        //the camera meat is here
#include "splash.h"        //splash and crash images in 8 bits (no compression, as I gain nothing)
#include "prettyborder.h"  //2 bpp image borders encoded in 8 bits with cheap RLE compression (very efficient)

//absence of the SD card leads to general failure of the code, so to test without, better deactivate the feature at compiling
#ifdef USE_SD
#include <SPI.h>  //for SD
#include <SD.h>   //for SD
#endif

//tft directly addresses the display, img is a memory buffer for sprites
//Here I create and update a giant 128*160 sprite in memory that I push to the screen when necessary, which is ultra fast way of rendering
#ifdef USE_TFT
#include <TFT_eSPI.h>                 //Include the graphics library (this includes the sprite functions)
TFT_eSPI tft = TFT_eSPI();            //Create object "tft"
TFT_eSprite img = TFT_eSprite(&tft);  //Create Sprite object "img" with pointer to "tft" object. The pointer is used by pushSprite() to push it onto the TFT
#endif

//////////////////////////////////////////////Setup, core 0/////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  //interface input/output, here I use the Pico SDK commands as they are faster (you can mix Arduino type and SDK type)
  gpio_init(PUSH);
  gpio_set_dir(PUSH, GPIO_IN);  //action button to record
  gpio_init(TLC);
  gpio_set_dir(TLC, GPIO_IN);  //timelapse<->regular camera mode
  gpio_init(DITHER);
  gpio_set_dir(DITHER, GPIO_IN);  //dithering with Bayer matrix
  gpio_init(HDR);
  gpio_set_dir(HDR, GPIO_IN);  //HDR mode
  gpio_init(LOCK);
  gpio_set_dir(LOCK, GPIO_IN);  //LOCK exposure
  gpio_init(LED);
  gpio_set_dir(LED, GPIO_OUT);  //green LED
  gpio_init(RED);
  gpio_set_dir(RED, GPIO_OUT);  //red LED
  gpio_init(INT);
  gpio_set_dir(INT, GPIO_OUT);  //internal LED

  //sensor input/output
  gpio_init(READ);
  gpio_set_dir(READ, GPIO_IN);
  gpio_init(CLOCK);
  gpio_set_dir(CLOCK, GPIO_OUT);
  gpio_init(RESET);
  gpio_set_dir(RESET, GPIO_OUT);
  gpio_init(LOAD);
  gpio_set_dir(LOAD, GPIO_OUT);
  gpio_init(SIN);
  gpio_set_dir(SIN, GPIO_OUT);
  gpio_init(START);
  gpio_set_dir(START, GPIO_OUT);

  //if you want to use GPIO22 as external trigger
  //gpio_init(INOUT);
  //gpio_set_dir(INOUT, GPIO_IN);  //to use with remote for example
  //gpio_set_dir(INOUT, GPIO_OUT); //to use with a flash for example

  //analog stuff
  adc_init();  //mandatory, without it stuck the camera, it must be called first
  //adc_gpio_init(VOUT); // I have no idea why, but this command has no effect, you have to use the command below
  adc_select_input(VOUT - 26);  //there are several ADC channels to choose from. 0 is GPIO26, 1 is GPIO27 and so on...

#ifdef USE_SERIAL  // serial is optional, only needed for debugging or interfacing with third party soft via USB cable
  Serial.begin(115200);
#endif

  init_sequence();  //Boot screen get stuck here with red flashing LED if any problem with SD or sensor to avoid further board damage
  //now if code arrives at this point, this means that sensor and SD card are connected correctly, we can go further
  difference_threshold = motion_detection_threshold * 128 * max_line * 255;  //trigger threshold for motion sensor
  x_min = (128 - x_box) / 2;                                                 //recalculate the autoexposure area limits with json config values
  y_min = (max_line - y_box) / 2;
  x_max = x_min + x_box;
  y_max = y_min + y_box;

  if (GBCAMERA_mode == 1) {       //Game Boy Camera strategy with variable gain and registers
    low_exposure_limit = 0x0010;  //minimum of the sensor for these registers, below there are verticals artifacts, see sensor documentation for details
    multiplier = 1.1;             //as GB camera uses only the upper voltage scale the autoexposure must be boosted a little in that case to be comfortable
    v_min = GB_v_min;             //those were chosen to maximize contrast
    v_max = GB_v_max;
    if (M64283FP == 1) {                     //strategy for the M64283FP sensor
      camReg1[4] = camReg1[4] | 0b00100000;  //the CL register must be HIGH to disable autocalibration, see https://github.com/Raphael-Boichot/Play-with-the-Mitsubishi-M64283FP-sensor
      camReg2[4] = camReg2[4] | 0b00100000;
      camReg3[4] = camReg3[4] | 0b00100000;
      camReg4[4] = camReg4[4] | 0b00100000;
      camReg5[4] = camReg5[4] | 0b00100000;

      camReg1[5] = camReg1[5] | 0b00010000;  //the OB register must be HIGH to disable autocalibration, see https://github.com/Raphael-Boichot/Play-with-the-Mitsubishi-M64283FP-sensor
      camReg2[5] = camReg2[5] | 0b00010000;
      camReg3[5] = camReg3[5] | 0b00010000;
      camReg4[5] = camReg4[5] | 0b00010000;
      camReg5[5] = camReg5[5] | 0b00010000;

      camReg1[7] = camReg1[7] | 0b01000000;  //Forcing E register to 50% 2D edge enhancement, see https://github.com/Raphael-Boichot/Mitsubishi-M64282FP-dashcam/blob/main/Docs%20and%20research/Image%20files/Register_E.png
      camReg2[7] = camReg2[7] | 0b01000000;
      camReg3[7] = camReg3[7] | 0b01000000;
      camReg4[7] = camReg4[7] | 0b01000000;
      camReg5[7] = camReg5[7] | 0b01000000;
    }
  } else {                              //regular strategy with fixed registers except C
    if (M64283FP == 1) {                //strategy for the M64283FP sensor
      low_exposure_threshold = 0x0021;  //recommended value by the datasheet of the M64283FP to avoid artifacts with 2D edge enhancement
      v_min = M64283FP_v_min;           //those were chosen to maximize contrast
      v_max = M64283FP_v_max;
    } else {                            //strategy for the M64282FP sensor
      low_exposure_threshold = 0x0030;  //recommended value by the datasheet of the M64282FP to avoid artifacts with 2D edge enhancement
      v_min = M64282FP_v_min;           //those were chosen to maximize contrast
      v_max = M64282FP_v_max;
    }
  }

#ifdef USE_SD
  ID_file_creator("/Dashcam_storage.bin");          //create a file on SD card that stores a unique file ID from 1 to 2^32 - 1 (in fact 1 to 99999)
  Next_ID = get_next_ID("/Dashcam_storage.bin");    //get the file number on SD card
  Next_dir = get_next_dir("/Dashcam_storage.bin");  //get the folder number on SD card
#endif

  pre_allocate_lookup_tables(lookup_serial, v_min, v_max);  //pre allocate tables for TFT and serial output auto contrast
  pre_allocate_lookup_tables(lookup_pico_to_GBD, 0, 255);   //pre allocate tables 0<->3.3V scale from pico to 0<->3.3V scale from MAC-GBD
  //yes, the flash ADC of the MAC-GBD is probably rated for 0<->3.3 volts (even probably 3.36V)
  if (PRETTYBORDER_mode > 0) {                 //border or no border ?
    pre_allocate_image_with_pretty_borders();  //pre allocate bmp data for image with borders
    Pre_allocate_bmp_header(160, 144);
  } else {
    Pre_allocate_bmp_header(128, max_line);
  }

  // initialize recording mode
  if (timelapse_list[0] >= 0) {  //there is a time in ms in the list, get it
    TIMELAPSE_mode = 1;
    TIMELAPSE_deadtime = timelapse_list[0];
  } else {  //there is -1 or -2 in the list, regular mode is called
    TIMELAPSE_mode = 0;
  }

  PRETTYBORDER_mode++;  //here the border is incremented, if device is booted during preset exposure (next loop), border will change
#ifndef USE_SNEAK_MODE
  gpio_put(RED, 1);
#endif
  Put_JSON_config("/config.json");  //Put data in json
// presets the exposure time before displaying to avoid unpleasing result, maybe be slow in the dark
#ifndef USE_SNEAK_MODE
  gpio_put(RED, 0);
#endif
  for (int i = 1; i < 15; i++) {
    take_a_picture();
    new_exposure = auto_exposure();          // self explanatory
    push_exposure(camReg, new_exposure, 1);  //update exposure registers C2-C3
  }
#ifndef USE_SNEAK_MODE
  gpio_put(RED, 1);
#endif
  PRETTYBORDER_mode--;
  Put_JSON_config("/config.json");  //Put data in json, if device is rebooted before, increment the border #
#ifndef USE_SNEAK_MODE
  gpio_put(RED, 0);
#endif
}  //end of setup

//////////////////////////////////////////////Main loop, core 0///////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  currentTime = millis();
  take_a_picture();                                   //data in memory for the moment, one frame
  image_TOKEN = 0;                                    //reset any attempt to take more than one picture without pushing a button or observe a difference
  detect_a_motion();                                  //does nothing if MOTION_sensor = 0
  edge_extraction();                                  //does nothing if FOCUS_mode = 0
  memcpy(CamData_previous, CamData, 128 * max_line);  //to deal with motion detection
  new_exposure = auto_exposure();                     //self explanatory

  if (FIXED_EXPOSURE_mode == 1) {
    new_exposure = FIXED_delay;
    clock_divider = FIXED_divider;
  }
  if (LOCK_exposure == 0) {
    push_exposure(camReg, new_exposure, 1);  //update exposure registers C2-C3
  }

  /////////////////////////////////////////
  if (gpio_get(LOCK) == 1) {
    LOCK_exposure = !LOCK_exposure;  //self explanatory
    short_fancy_delay();
  }
  if (gpio_get(HDR) == 1) {
    HDR_mode = !HDR_mode;  //self explanatory
    short_fancy_delay();
  }
  if (gpio_get(DITHER) == 1) {
    DITHER_mode = !DITHER_mode;  //self explanatory
    short_fancy_delay();
  }
  if ((gpio_get(TLC) == 1) & (recording == 0) & (image_TOKEN == 0)) {  //Change regular camera<->timelapse mode, but only when NOT recording
    rank_timelapse++;
    MOTION_sensor = 0;
    MOTION_sensor_counter = 0;
    if (rank_timelapse >= 8) {
      rank_timelapse = 0;
    }
    if (timelapse_list[rank_timelapse] >= 0) {
      TIMELAPSE_mode = 1;
      TIMELAPSE_deadtime = timelapse_list[rank_timelapse];
    } else {
      TIMELAPSE_mode = 0;
    }
    //there is "-2" in the list, motion sensor mode is called
    if (timelapse_list[rank_timelapse] == -2) {
      currentTime_MOTION = millis();
      MOTION_sensor = 1;
      difference = 0;
      Next_dir++;

#ifdef USE_SD
      store_next_ID("/Dashcam_storage.bin", Next_ID, Next_dir);  //store last known file/directory# to SD card
#endif
    }
    short_fancy_delay();
  }
  ////////////////////////////////////////

  if (DITHER_mode == 1) {
    memcpy(Dithering_patterns, Dithering_patterns_regular, sizeof(Dithering_patterns_regular));

    if (GBCAMERA_mode == 1) {
      if (dithering_strategy[register_strategy] == 1) {
        memcpy(Dithering_patterns, Dithering_patterns_high, sizeof(Dithering_patterns_high));
      }
      if (dithering_strategy[register_strategy] == 0) {
        memcpy(Dithering_patterns, Dithering_patterns_low, sizeof(Dithering_patterns_low));
      }
    }

    pre_allocate_Bayer_tables();  //just reordering the Game Boy Camera dithering registers into 3 square matrices
    Dither_image();               //apply dithering
  }

#ifdef USE_SERIAL
  dump_data_to_serial();  //dump raw data to serial in ASCII for debugging - you can use the Matlab code ArduiCam_Matlab.m into the repo to probe the serial and plot images
#endif

#ifdef USE_TFT
  img.fillSprite(TFT_BLACK);  //prepare the "video ram" (big sprite)
  for (unsigned char x = 0; x < 128; x++) {
    for (unsigned char y = 0; y < 120; y++) {
      if (DITHER_mode == 1) {
        pixel_TFT_RGB565 = lookup_TFT_RGB565[BayerData[x + y * 128]];  //BayerData includes auto-contrast
      } else {
        pixel_TFT_RGB565 = lookup_TFT_RGB565[lookup_serial[CamData[x + y * 128]]];  //lookup_serial is autocontrast
      }
      if (FOCUS_mode == 1) {
        if (EdgeData[x + y * 128] > FOCUS_threshold) {
          pixel_TFT_RGB565 = (pixel_TFT_RGB565 | 0b1111100000000000);  //overlay with RED
        }
        //img.drawPixel(x, y + display_offset, lookup_TFT_RGB565[EdgeData[x + y * 128]]);  //lookup_serial is autocontrast
      }
      img.drawPixel(x, y + display_offset, pixel_TFT_RGB565);  //BayerData includes auto-contrast
    }
  }
#endif

  if ((recording == 1) & (image_TOKEN == 0)) {  //prepare for recording in timelapse mode
#ifdef USE_TFT
    img.setTextColor(TFT_RED);
    img.setCursor(0, 8);
    img.println("Recording...");
    img.setCursor(84, 8);
    sprintf(files_on_folder_string, "%X/%X", files_on_folder, max_files_per_folder);
    img.println(files_on_folder_string);
    display_other_informations();
    img.pushSprite(x_ori, y_ori);  //dump image to display
#endif
    if ((currentTime - previousTime) > TIMELAPSE_deadtime) {  //Wait for deadtime set in config.txt
      recording_loop();                                       //here is the juicy stuff
    }
    //if (TIMELAPSE_deadtime > 10000) {
    sleep_ms(min(TIMELAPSE_deadtime / 10, 1000));  //for timelapses with long deadtimes, no need to constantly spam the sensor for autoexposure, saves battery
    //}
  }  //end of recording loop for timelapse

  if ((TIMELAPSE_mode == 0) & (gpio_get(PUSH) == 1) & (MOTION_sensor == 0)) {  //camera mode acts like if user requires just one picture

#ifdef USE_SD
    Next_ID = get_next_ID("/Dashcam_storage.bin");    //get the file number on SD card
    Next_dir = get_next_dir("/Dashcam_storage.bin");  //get the folder number on SD card, just to store it in memory and rewrite it at the end
#endif

    image_TOKEN = 1;  //gives a single token for recording
  }

  if ((image_TOKEN == 1) & (recording == 0)) {  //prepare for recording one shot

#ifdef USE_TFT
    img.setTextColor(TFT_RED);
    img.setCursor(0, 8);
    img.println("Recording image");
    if (MOTION_sensor == 1) {
      img.setCursor(100, 8);
      img.println(MOTION_sensor_counter, HEX);
    }
    display_other_informations();
    img.pushSprite(x_ori, y_ori);  //dump image to display
#endif

    if ((SLIT_SCAN_mode == 0) | (MOTION_sensor == 1)) {  //forbid slit scan mode in motion sensor mode !
      recording_loop();                                  //record a single image
    } else {                                             //enter slit scan own recording loop
      recording_slit_scan();
    }

#ifdef USE_SD
    store_next_ID("/Dashcam_storage.bin", Next_ID, Next_dir);  //store last known file/directory# to SD card
#endif

    if (MOTION_sensor == 0) {
      delay(200);  //long enough for debouncing, fast enough for a decent burst mode
    } else {
      delay(25);  //avoid image artifacts due to 5 volts instability after recording, not supposed to happens but...
    }
  }  //end of recording loop for regular camera mode

  if ((recording == 0) & (image_TOKEN == 0)) {  //just put informations to the display

#ifdef USE_TFT
    img.setTextColor(TFT_GREEN);
    img.setCursor(0, 8);
    img.fillRect(0, 8, 128, 8, TFT_BLACK);
    img.println("Display Mode");
    if (MOTION_sensor == 1) {
      img.setCursor(100, 8);
      img.println(MOTION_sensor_counter, HEX);
    }
    display_other_informations();
    img.pushSprite(x_ori, y_ori);  //dump image to display
#endif
  }

  if (TIMELAPSE_mode == 1) {
    if ((gpio_get(PUSH) == 1) & (recording == 1)) {  //we want to stop recording
      recording = 0;
      files_on_folder = 0;
      short_fancy_delay();  //blinks red to acknowledge action
    }
    if ((gpio_get(PUSH) == 1) & (recording == 0)) {  //we want to record: get file/directory#
      Next_dir++;                                    //update next directory

#ifdef USE_SD
      store_next_ID("/Dashcam_storage.bin", Next_ID, Next_dir);  //store last known file/directory# to SD card
#endif

      recording = 1;  //keep on timelapse
      previousTime = currentTime;
      short_fancy_delay();
    }
  }
}  //end of main loop

//////////////////////////////////////////////Sensor stuff///////////////////////////////////////////////////////////////////////////////////////////
void take_a_picture() {
  camReset();         //resets the sensor
  camSetRegisters();  //Send 8 registers to the sensor, enough for driving the M64282FP/M64283FP
  camReadPicture();   //get pixels, dump them in CamData
  camReset();         //probably not usefull but who knows...
}

double auto_exposure() {
  double exp_regs, new_regs;                      //doubles are luxury but we are on a powerfull device and 8 bits arithmetics is not always fun
  unsigned char setpoint = (v_max + v_min) >> 1;  //set point is just voltage mid scale, why not...
  unsigned int accumulator = 0;                   //exposure integrator
  unsigned char least_change = 1;
  unsigned int counter = 0;
  int i = 0;

  for (unsigned char y = 1; y <= max_line; y++) {
    for (unsigned char x = 1; x <= 128; x++) {
      if (((y >= y_min) && (y <= y_max)) && ((x >= x_min) && (x <= x_max))) {  //we check only a centered box and not the whole image
        accumulator = accumulator + CamData[i];                                //accumulate the mean gray level, but only from line 0 to 120 as bottom of image have artifacts
        counter++;                                                             //I use a counter in order to be sure I do not forget a line of pixels in the conditions (lazy)
      }
      i++;
    }
  }

  mean_value = accumulator / (counter);
  error = setpoint - mean_value;  //so in case of deviation, registers 2 and 3 are corrected
  //this part is very similar to what a Game Boy Camera does, except that it does the job with only bitshift operators and in more steps.
  //you can check https://github.com/untoxa/gb-photo/blob/7bde50533d4e7d30d5203d7adabe244513c279c9/src/state_camera.c#L1009
  //to see a "real" autoexposure algortihm of my own in full 8 bits arithmetics
  //Here we can use 32 bits variables for ease of programming. The bigger the error is, the bigger the correction on exposure is, simple.

  exp_regs = camReg[2] * 256 + camReg[3];  //I know, it's a shame to use a double here but we have quite plenty of ram
  new_regs = exp_regs;
  //These parameters are bullet proofed and were tuned during hours of tests, you may not find better ones
  if (error > 80) new_regs = exp_regs * (2 * multiplier);  //raw tuning
  if (error < -80) new_regs = exp_regs / (2 * multiplier);
  if ((error <= 80) & (error >= 50)) new_regs = exp_regs * (1.3 * multiplier);  //yes floating point, I know...
  if ((error >= -80) & (error <= -50)) new_regs = exp_regs / (1.3 * multiplier);
  if ((error <= 50) & (error >= 30)) new_regs = exp_regs * (1.1 * multiplier);  //fine tuning
  if ((error >= -50) & (error <= -30)) new_regs = exp_regs / (1.1 * multiplier);
  if ((error <= 30) & (error >= 10)) new_regs = exp_regs * (1.03 * multiplier);  //very fine tuning
  if ((error >= -30) & (error <= -10)) new_regs = exp_regs / (1.03 * multiplier);

  if (exp_regs > 0x00FF) {
    least_change = 0x0F;  //least change must increase if exposure time is high
  }
  if (exp_regs > 0x0FFF) {
    least_change = 0xFF;  //least change must increase a lot if exposure time is very high
  }

  if ((M64283FP == 1) & (accumulator < 150000)) {  //Sensor saturated !
    new_regs = low_exposure_threshold;             //anti-glare strategy for the dodgy M64283FP sensor...
  }

  if ((error <= 10) & (error >= 5)) new_regs = exp_regs + least_change;    //this level is critical to avoid flickering in full sun, 3-4 is nice too
  if ((error >= -10) & (error <= -5)) new_regs = exp_regs - least_change;  //this level is critical to avoid flickering in full sun,  3-4 is nice too
  if (new_regs < 0) {                                                      //maybe over precautious but who knows...
    new_regs = 0;                                                          //this must never happened
  }
  exposure_error = error;  //just to pass the variable to push_exposure
  return new_regs;
}

void push_exposure(unsigned char camReg[8], double current_exposure, double factor) {
  double new_regs;
  unsigned short int storage_regs;
  unsigned char old_strategy;
  unsigned char temp_camReg[8];
  new_regs = current_exposure * factor;     //usefull for HDR mode only, either factor is always = 1
  storage_regs = int(new_regs);             //enforce register type
  if (storage_regs < low_exposure_limit) {  //minimum of the sensor for these registers, below there are verticals artifacts, see sensor documentation for details
    storage_regs = low_exposure_limit;      //minimum of the sensor for these registers, below there are verticals artifacts, see sensor documentation for details
  }
  if (new_regs > 0xFFFF) {  //maximum of the sensor, about 1 second
    if (NIGHT_mode == 1) {
      clock_divider = clock_divider + 1;  //the sensor is downclocked to increase exposure in this mode
    }
    storage_regs = 0xFFFF;
  }
  if (NIGHT_mode == 1) {
    if (storage_regs < 0x1000) {
      clock_divider = 1;  //Normal situation is to be always 1, so that clock is about 1MHz. This looks very sketchy but it works well
    }
  }

  if (GBCAMERA_mode == 1) {  //Game Boy Camera strategy, found by datalogging a real camera
    old_strategy = register_strategy;
    if (storage_regs < 0x0030) {
      register_strategy = 1;
    }
    if ((storage_regs >= 0x0030) & (storage_regs < 0x0D80)) {
      register_strategy = 2;
    }
    if ((storage_regs >= 0x0D80) & (storage_regs < 0x3500)) {
      register_strategy = 3;
    }
    if ((storage_regs >= 0x3500) & (storage_regs < 0x8500)) {
      register_strategy = 4;
    }
    if ((storage_regs > 0x8500)) {
      register_strategy = 5;
    }

    //this part forces the camera to not jump in register stategy when exposure is changing very slowly
    //I think this is what a real camera does too, I've tried many different tricks to avoid jittering between strategies but this one works well
    overshooting = 0;
    if ((abs(exposure_error) < jittering_threshold) & (exposure_error > 0))  //Avoid register jittering
    {
      if (!(old_strategy == register_strategy)) {
        overshooting = 1;  //this just change the color of an indicator on screen
      }
      register_strategy = old_strategy;  //keep the previous registers because doing fine tuning
    }

    switch (register_strategy) {
      case 1:
        memcpy(temp_camReg, camReg1, 8);
        break;
      case 2:
        memcpy(temp_camReg, camReg2, 8);
        break;
      case 3:
        memcpy(temp_camReg, camReg3, 8);
        break;
      case 4:
        memcpy(temp_camReg, camReg4, 8);
        break;
      case 5:
        memcpy(temp_camReg, camReg5, 8);
        break;
      default:
        {};  //do nothing
    }
  } else {  // this in NOT Game Boy Camera mode so a single strategy register is chosen
    if (M64283FP == 1) {
      memcpy(temp_camReg, camReg_M64283FP, 8);  //regular single strategy mode
    } else {
      memcpy(temp_camReg, camReg_M64282FP, 8);  //regular single strategy mode
    }
    if (storage_regs < low_exposure_threshold) {
      temp_camReg[1] = temp_camReg[1] & 0b00011111;  //cancel 2D border enhancement, to avoid jailbars at low exposure time
    }
  }
  //regsiters are updated now
  camReg[0] = temp_camReg[0];
  camReg[1] = temp_camReg[1];
  camReg[2] = storage_regs >> 8;
  camReg[3] = storage_regs;  //forcing a int into a char suppresses MSBs up to 8
  camReg[4] = temp_camReg[4];
  camReg[5] = temp_camReg[5];
  camReg[6] = temp_camReg[6];
  camReg[7] = temp_camReg[7];

  //this part is more experimental than usefull but I decide to let it just for fun
  if ((BORDER_mode == 1) && (DITHER_mode == 0)) {  //enforce 2D border enhancement only in non dither mode
    camReg[1] = camReg[1] | 0b11100000;
  }
  if ((SMOOTH_mode == 1) && (DITHER_mode == 0)) {  //cancel 2D border enhancement only in non dither mode
    camReg[1] = camReg[1] & 0b00011111;
  }
}

unsigned int get_exposure(unsigned char camReg[8]) {
  double exp_regs;
  exp_regs = camReg[2] * 256 + camReg[3];  //
  return exp_regs;
}

void camDelay()  //Allow a lag in processor cycles to maintain signals long enough, critical for exposure time, sensor must be clocked at 1 MHz MAXIMUM (can be less, see nigth mode)
{
  for (int i = 0; i < cycles; i++) NOP;
}

void camSpecialDelay()  //Allow an extra lag in processor cycles during exposure to allow night mode
{
  for (int i = 0; i < cycles * clock_divider; i++) NOP;
}

void camInit()  //Initialise the IO ports for the camera, see datasheets of the sensors, timing is not critical here
{
  gpio_put(CLOCK, 0);
  gpio_put(RESET, 1);
  gpio_put(LOAD, 0);
  gpio_put(START, 0);
  gpio_put(SIN, 0);
}

void camReset()  //Sends a RESET pulse to sensor, see datasheets of the sensors, timing is not critical here
{
  gpio_put(CLOCK, 1);
  camDelay();
  gpio_put(CLOCK, 0);
  camDelay();
  gpio_put(RESET, 0);
  gpio_put(CLOCK, 1);
  camDelay();
  gpio_put(RESET, 1);
}

void camSetRegisters()  //Sets the sensor 8 registers, can be 10 for the M64283FP sensor
{
  for (int reg = 0; reg < 8; ++reg) {
    camSetReg(reg, camReg[reg]);
  }
}

void camSetReg(unsigned char regaddr, unsigned char regval)  //Sets one of the 8 8-bit registers in the sensor, from 0 to 7, in this order
{                                                            //GB camera uses another order but sensor do not mind
  unsigned char bitmask;
  for (bitmask = 0x4; bitmask >= 0x1; bitmask >>= 1) {  //Write 3-bit address.
    gpio_put(CLOCK, 0);
    camDelay();
    gpio_put(LOAD, 0);  //ensure load bit is cleared from previous call
    if (regaddr & bitmask) {
      gpio_put(SIN, 1);  //Set the SIN bit
    } else {
      gpio_put(SIN, 0);
    }
    gpio_put(CLOCK, 1);
    camDelay();
    gpio_put(SIN, 0);  //set the SIN bit low
  }
  for (bitmask = 128; bitmask >= 1; bitmask >>= 1) {  //Write the 8-bits register
    gpio_put(CLOCK, 0);
    camDelay();
    if (regval & bitmask) {
      gpio_put(SIN, 1);  //set the SIN bit
    } else {
      gpio_put(SIN, 0);
    }
    if (bitmask == 1) {
      gpio_put(LOAD, 1);  //Assert load at rising edge of CLOCK
    }
    gpio_put(CLOCK, 1);
    camDelay();
    gpio_put(SIN, 0);
  }
}

void camReadPicture()  //Take a picture, read it and store it
{
  unsigned int pixel;  //Buffer for pixel read in
  int x, y;
  int subcounter = 0;
  masked_pixels = 0;
  //Camera START sequence
  gpio_put(CLOCK, 0);
  camDelay();  //ensure load bit is cleared from previous call
  gpio_put(LOAD, 0);
  gpio_put(START, 1);  //START rises before CLOCK
  camDelay();
  gpio_put(CLOCK, 1);
  camDelay();
  gpio_put(START, 0);  //START valid on rising edge of CLOCK, so can drop now
  camDelay();
  gpio_put(CLOCK, 0);
  camDelay();

#ifndef USE_SNEAK_MODE
  gpio_put(LED, 1);
#endif

  bool skip_loop = 0;
  previousTime_exp = millis();
  while (1) {  //Wait for READ to go high, this is the loop waiting for exposure
    gpio_put(CLOCK, 1);
    camSpecialDelay();
    if (gpio_get(READ) == 1) {
      break;  //READ goes high with rising CLOCK, normal ending
    }
    if (((gpio_get(PUSH) == 1) & (image_TOKEN == 0)) | (gpio_get(HDR) == 1) | (gpio_get(DITHER) == 1) | (gpio_get(TLC) == 1) | (gpio_get(LOCK) == 1)) {  //any button is pushed, skip sensor stuff
      skip_loop = 1;
      camReset();
      break;  //we want to do something, skip next steps
    }
    gpio_put(CLOCK, 0);
    camSpecialDelay();
  }
  currentTime_exp = millis() - previousTime_exp;  //to dislay the real exposure time, not the registers
  gpio_put(LED, 0);
  if (skip_loop == 0) {  //procedure not interrupted by user
    for (y = 0; y < 128; y++) {
      for (x = 0; x < 128; x++) {
        gpio_put(CLOCK, 0);
        camDelay();
        pixel = adc_read();                //The ADC is 12 bits, this sacrifies the 4 least significant bits to simplify transmission
        CamData[subcounter] = pixel >> 4;  //record only 8 bits

#ifdef DEBAGAME_MODE  //black pixel line is the first line on the M64283FP and the 4 last in the M64282FP, do not ask me why...
        if (M64283FP == 1) {
          if (y == 0) {
            masked_pixels = masked_pixels + CamData[subcounter];  //used to calculate the black level voltage
          }
        } else {
          if (y == 127) {                                         //I take just one line with the 82FP to stay consistent with the 83FP
            masked_pixels = masked_pixels + CamData[subcounter];  //used to calculate the black level voltage
            //funfact: these pixels are always stuck at the maximal saturation voltage.
          }
        }
#endif
        subcounter = subcounter + 1;
        gpio_put(CLOCK, 1);
        camDelay();
      }  // end for x
    }    /* for y */

    while (gpio_get(READ) == 1) {  //Go through the remaining rows, not sure if mandatory as we got all pixels but why not
      gpio_put(CLOCK, 0);
      camDelay();
      gpio_put(CLOCK, 1);
      camDelay();
    }
    gpio_put(CLOCK, 0);
    camDelay();
  }
}

bool camTestSensor()  //dummy cycle faking to take a picture, if it's not able to go through the whole cycle, the camera will stop with error message
{                     //it basically checks if READ is able to change at the good moment during the sequence, if not, sensor not responding
  bool sensor_OK = 1;
  int x, y;
  int currentTime;
  //Camera START sequence
  gpio_put(CLOCK, 0);
  camDelay();  //ensure load bit is cleared from previous call
  gpio_put(LOAD, 0);
  gpio_put(START, 1);  //START rises before CLOCK
  gpio_put(CLOCK, 1);
  camDelay();
  gpio_put(START, 0);  //START valid on rising edge of CLOCK, so can drop now
  gpio_put(CLOCK, 0);
  camDelay();
#ifndef USE_SNEAK_MODE
  gpio_put(LED, 1);
#endif
  currentTime = millis();
  while (1) {  //Wait for READ to go high
    gpio_put(CLOCK, 1);
    camDelay();
    if (gpio_get(READ) == 1) {
      break;  //READ goes high with rising CLOCK, everything is OK
    }
    if ((millis() - currentTime) > 1000) {
      sensor_OK = 0;
      break;  //the sensor does not respond after 1 second = not connected
    }
    gpio_put(CLOCK, 0);
    camDelay();
  }
  camDelay();
  gpio_put(LED, 0);
  for (y = 0; y < 128; y++) {
    for (x = 0; x < 128; x++) {
      gpio_put(CLOCK, 0);
      camDelay();
      //adc_read();//doing nothing, just performing a loop
      camDelay();
      gpio_put(CLOCK, 1);
      camDelay();
    }  // end for x
  }    /* for y */
  currentTime = millis();
  while (gpio_get(READ) == 1) {  //Go through the remaining rows, but READ can stay high due to level shifter design
    gpio_put(CLOCK, 0);
    camDelay();
    gpio_put(CLOCK, 1);
    camDelay();
    if ((millis() - currentTime) > 1000) {
      sensor_OK = 0;
      break;  //the sensor does not respond after 1 seconds = not connected
    }
  }
  gpio_put(CLOCK, 0);
  camDelay();
  return sensor_OK;
}

void detect_a_motion() {
  if (MOTION_sensor == 1) {
    difference = 0;
    for (int i = 0; i < 128 * max_line; i++) {
      difference = difference + abs(CamData[i] - CamData_previous[i]);  //calculate the image difference
    }
    if ((difference > difference_threshold) & ((millis() - currentTime_MOTION) > delay_MOTION)) {
      image_TOKEN = 1;  //gives a token to record one image
      MOTION_sensor_counter++;
    }
  }
}

void edge_extraction() {                    //the sensor can do this too but it requires taking two images which is slower than online software implementation
  if (FOCUS_mode == 1) {                    //first pass
    memset(EdgeData, 0, sizeof(EdgeData));  //clean the data array
    int offset = 0;
    for (int y = 0; y < 128; y++) {
      for (int x = 0; x < 128; x++) {
        //formula from sensor datasheet, just a border extraction kernel
        if ((x > 0) & (x < 128) & (y > 0) & (y < 128)) {
          EdgeData[offset] = abs(4 * CamData[offset] - (CamData[offset - 128] + CamData[offset + 128] + CamData[offset - 1] + CamData[offset + 1]));
        }
        offset++;
      }          // end for x
    }            /* for y */
    offset = 0;  //second pass to eliminate pure noise, not mandatory but fancier
    for (int y = 0; y < 128; y++) {
      for (int x = 0; x < 128; x++) {  // we search for weak threshold in Moore neighborhood
        if ((x > 0) & (x < 128) & (y > 0) & (y < 128)) {
          if ((EdgeData[offset - 127] < FOCUS_threshold) & (EdgeData[offset - 128] < FOCUS_threshold) & (EdgeData[offset - 129] < FOCUS_threshold) & (EdgeData[offset - 1] < FOCUS_threshold) & (EdgeData[offset + 127] < FOCUS_threshold) & (EdgeData[offset + 128] < FOCUS_threshold) & (EdgeData[offset + 129] < FOCUS_threshold) & (EdgeData[offset + 1] < FOCUS_threshold)) {
            EdgeData[offset] = 0;
          }
        }
        offset++;
      }  // end for x
    }    /* for y */
  }
}

//////////////////////////////////////////////Output stuff///////////////////////////////////////////////////////////////////////////////////////////
void recording_loop() {
  if ((RAW_recording_mode == 0) | (image_TOKEN == 1)) {
    Next_ID++;  //update the file number, but not in movie maker mode
  }
  previousTime = currentTime;
  if (TIMELAPSE_mode == 1) {
    if (RAW_recording_mode == 0) {
      sprintf(storage_file_name, "/TL/%05d/%07d.bmp", Next_dir, Next_ID);  //update filename
    }
    if (RAW_recording_mode == 1) {
      sprintf(storage_file_name, "/TL/%05d.raw", Next_dir);  //update filename
    }
  }
  if (TIMELAPSE_mode == 0) {
    if (MOTION_sensor == 0) {
      sprintf(storage_file_name, "/Camera/%07d.bmp", Next_ID);  //update filename
    }
    if ((MOTION_sensor == 1) & (RAW_recording_mode == 0)) {
      sprintf(storage_file_name, "/MS/%05d/%07d.bmp", Next_dir, Next_ID);  //update filename
    }
    if ((MOTION_sensor == 1) & (RAW_recording_mode == 1)) {
      sprintf(storage_file_name, "/MS/%05d.raw", Next_dir);  //update filename
    }
  }

  if (HDR_mode == 1) {  //default is 8 pictures, beware of modifying the code in case of change

#ifndef USE_SNEAK_MODE
    gpio_put(RED, 1);
#endif

    memset(HDRData, 0, sizeof(HDRData));      //clean the HDR data array
    current_exposure = get_exposure(camReg);  //store the current exposure register for later
    for (int i = 0; i < num_HDR_images; i++) {
      push_exposure(camReg, current_exposure, exposure_list[i]);  //vary the exposure
      take_a_picture();
      for (int i = 0; i < 128 * 128; i++) {
        HDRData[i] = HDRData[i] + CamData[i];  //sum data
      }
    }
    //now time to average all that shit
    for (int i = 0; i < 128 * 128; i++) {
      CamData[i] = HDRData[i] / num_HDR_images;  //do the average
    }
    push_exposure(camReg, current_exposure, 1);  //rewrite the old register stored before
  }

  if (DITHER_mode == 1) {
    //Dither_image();
    for (int i = 0; i < 128 * 128; i++) {
      BmpData[i] = BayerData[i];  //to get data with dithering (dithering includes auto-contrast)
    }
  } else {
    for (int i = 0; i < 128 * 128; i++) {
      BmpData[i] = lookup_serial[CamData[i]];  //to get data with autocontrast
    }
  }

  if (PRETTYBORDER_mode > 0) {
    make_image_with_pretty_borders();
  }

#ifndef USE_SNEAK_MODE
  gpio_put(RED, 1);
#endif

  dump_data_to_SD_card();  //////////////////////cannot move this to core 1 without bug, set aside for the moment

#ifdef USE_SD
  if (TIMELAPSE_mode == 1) {
    files_on_folder++;
    if (files_on_folder == max_files_per_folder) {  //because up to 1000 files per folder stalling or errors in writing can happens
      files_on_folder = 0;
      store_next_ID("/Dashcam_storage.bin", Next_ID, Next_dir);  //in case of crash...
      if (RAW_recording_mode == 0) {
        Next_dir++;
      }
    }
  }
#endif

#ifndef USE_SNEAK_MODE
  gpio_put(RED, 0);
#endif
}

void recording_slit_scan() {
  delay(2000);
  bool enhance_temp = SMOOTH_mode;
  SMOOTH_mode = 1;                                            //enforce smooth mode to void noise streaks
  Next_ID++;                                                  //update the file number, but not in movie maker mode
  sprintf(storage_file_name, "/Slitscan/%07d.bmp", Next_ID);  //update filename
  File Datafile = SD.open(storage_file_name, FILE_WRITE);
  if (LOCK_exposure == 0) {
    for (int i = 0; i < 10; i++) {  //pre set exposure with new registers
      //camReg[1] = camReg[1] & 00011111;  //deactivates image enhancement to reduce noise
      take_a_picture();
      new_exposure = auto_exposure();          // self explanatory
      push_exposure(camReg, new_exposure, 1);  //update exposure registers C2-C3
    }
  }
#ifndef USE_SNEAK_MODE
  gpio_put(RED, 1);
#endif
  long int slit_offset = 0;
  for (int y = 0; y < 1078; y++) {
    Datafile.write(0x66);  //reserve the space for BMP header
  }
  //now registers are locked
  while (!(gpio_get(PUSH) == 1) && (slit_offset < 0xFFFF)) {
    int offset = 0;
    //camReg[1] = camReg[1] & 00011111;  //deactivates image enhancement to reduce noise
    take_a_picture();
    for (int y = 0; y < 128; y++) {
      for (int x = 0; x < 128; x++) {
        if (x == 64) {
          SlitData[y] = lookup_serial[CamData[offset]];  //to reduce noise
        }
        offset++;
      }  // end for x
    }    /* for y */
    slit_offset++;
    Datafile.write(SlitData, max_line_for_recording);
  }

  Pre_allocate_bmp_header(max_line_for_recording, slit_offset);  //number of lines will be updated at the end
  Datafile.seek(0);
  Datafile.write(BMP_header_generic, 54);     //fixed header
  Datafile.write(BMP_indexed_palette, 1024);  //indexed RGB palette
  Datafile.close();
  //now restoring back the current BMP header
  if (PRETTYBORDER_mode > 0) {                 //border or no border ?
    pre_allocate_image_with_pretty_borders();  //pre allocate bmp data for image with borders
    Pre_allocate_bmp_header(160, 144);
  } else {
    Pre_allocate_bmp_header(128, max_line);
  }
  store_next_ID("/Dashcam_storage.bin", Next_ID, Next_dir);  //in case of crash...
  SMOOTH_mode = enhance_temp;
#ifndef USE_SNEAK_MODE
  gpio_put(RED, 0);
#endif
}

void pre_allocate_lookup_tables(unsigned char lookup_serial[256], unsigned char v_min, unsigned char v_max) {
  double gamma_pixel;
  for (int i = 0; i < 256; i++) {  //building the autocontrat table lookup_serial
    if (i < v_min) {
      lookup_serial[i] = 0x00;  //must never happen
    }
    if ((i >= v_min) & (i <= v_max)) {
      gamma_pixel = ((i - double(v_min)) / (double(v_max) - double(v_min))) * 255;
      lookup_serial[i] = int(gamma_pixel);
    }
    if (i > v_max) {
      lookup_serial[i] = 0xFF;  //must never happen
    }
  }
}

void pre_allocate_Bayer_tables() {
  //this reorganizes the thresholding matrices from Game Boy Camera registers to "Bayer like" matrices in reading order (natural order in camera is weird)
  int counter = 0;
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      Bayer_matDG_B[x + 4 * y] = Dithering_patterns[counter];
      counter = counter + 1;
      Bayer_matLG_DG[x + 4 * y] = Dithering_patterns[counter];
      counter = counter + 1;
      Bayer_matW_LG[x + 4 * y] = Dithering_patterns[counter];
      counter = counter + 1;
    }
  }
}

void Dither_image() {  //dithering algorithm, I very struggled to understand it but OK now thanks to https://herrzatacke.github.io/dither-pattern-gen/
  char pixel, pixel_out;
  int counter = 0;
  for (int y = 0; y < max_line; y++) {
    for (int x = 0; x < 128; x++) {
      //pixel = CamData[counter];//non auto_contrasted values, may range between 0 and 255
      if (GBCAMERA_mode == 0) {
        pixel = lookup_serial[CamData[counter]];  //autocontrast is applied here, may range between 0 and 255
      }
      if (GBCAMERA_mode == 1) {
        pixel = lookup_pico_to_GBD[CamData[counter]];  //raw value, in this mode the dithering does the autocontrast
      }
      if (pixel < Bayer_matDG_B[(x & 3) + 4 * (y & 3)]) {
        pixel_out = Dithering_palette[0];
      }
      if ((pixel >= Bayer_matDG_B[(x & 3) + 4 * (y & 3)]) & (pixel < Bayer_matLG_DG[(x & 3) + 4 * (y & 3)])) {
        pixel_out = Dithering_palette[1];
      }
      if ((pixel >= Bayer_matLG_DG[(x & 3) + 4 * (y & 3)]) & (pixel < Bayer_matW_LG[(x & 3) + 4 * (y & 3)])) {
        pixel_out = Dithering_palette[2];
      }
      if (pixel >= Bayer_matW_LG[(x & 3) + 4 * (y & 3)]) {
        pixel_out = Dithering_palette[3];
      }
      BayerData[counter] = pixel_out;
      counter++;
    }
  }
}

void make_image_with_pretty_borders() {
  int image_counter = 8 * 128;              //remove the 8 first pixel line like a Game Boy Camera
  int pretty_image_counter = 16 * 160;      //beginning of first pixel to fill
  for (int line = 0; line < 112; line++) {  //line counter, copy lines 8 to 120
    pretty_image_counter = pretty_image_counter + 16;
    for (int x = 0; x < 128; x++) {  //column counter
      BigBmpData[pretty_image_counter] = BmpData[image_counter];
      pretty_image_counter++;
      image_counter++;
    }
    pretty_image_counter = pretty_image_counter + 16;
  }
}

void pre_allocate_image_with_pretty_borders() {  //current border is stored in ram at boot as a big array, so borders can't be changed dynamically
                                                 //it's a design choice to increase velocity by sacrifying some ram
  memset(BigBmpData, 0, sizeof(BigBmpData));     //clean the BigBmpData data array
  int counter = 0;
  int index = 0;
  char pixel = 0;
  char number_pixel = 0;
  while (counter < 160 * 144) {  //el cheapo RLE decoder, totally pointless so totally rad
    switch (PRETTYBORDER_mode) {
      case 1:
        number_pixel = prettyborder_1[index];
        pixel = Dithering_palette[prettyborder_1[index + 1]];
        break;
      case 2:
        number_pixel = prettyborder_2[index];
        pixel = Dithering_palette[prettyborder_2[index + 1]];
        break;
      case 3:
        number_pixel = prettyborder_3[index];
        pixel = Dithering_palette[prettyborder_3[index + 1]];
        break;
      case 4:
        number_pixel = prettyborder_4[index];
        pixel = Dithering_palette[prettyborder_4[index + 1]];
        break;
      case 5:
        number_pixel = prettyborder_5[index];
        pixel = Dithering_palette[prettyborder_5[index + 1]];
        break;
      case 6:
        number_pixel = prettyborder_6[index];
        pixel = Dithering_palette[prettyborder_6[index + 1]];
        break;
      case 7:
        number_pixel = prettyborder_7[index];
        pixel = Dithering_palette[prettyborder_7[index + 1]];
        break;
      default:
        BigBmpData[counter] = 0;
    }

    for (int i = 0; i < number_pixel; i++) {
      BigBmpData[counter] = pixel;
      counter = counter + 1;
    }
    index = index + 2;  //because data are encoded as {number of pixels, color, number of pixels, colors, etc}, 160x144 pixels
  }
}

void dump_data_to_serial() {  //output data directly to the serial monitor; only for debug
  char pixel;
  for (int i = 0; i < 128 * 128; i++) {
    pixel = lookup_serial[CamData[i]];  //to get data with autocontrast
    //pixel = CamData[i]; //to get raw data from sensor without autocontrast
    if (pixel <= 0x0F) {
      Serial.print('0');
    }
    Serial.print(pixel, HEX);
    Serial.print(" ");
  }
  Serial.println("");
}

//////////////////////////////////////////////SD stuff///////////////////////////////////////////////////////////////////////////////////////////
void ID_file_creator(const char* path) {  //from fresh SD, device needs a "secret" binary storage file
//this file may never be erased and is accessed frequently as it counts all images recorded, this why it is not embedded in json stuff
#ifdef USE_SD
  uint8_t buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  if (!SD.exists(path)) {
    File Datafile = SD.open(path, FILE_WRITE);
    //start from a fresh install on SD
    Datafile.write(buf, 8);
    Datafile.close();
  }
#endif
}

unsigned long get_next_ID(const char* path) {  //get the next file #
#ifdef USE_SD
  uint8_t buf[4];
  File Datafile = SD.open(path);
  Datafile.read(buf, 4);
  Next_ID = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]));
  Datafile.close();
#endif
  return Next_ID;
}

unsigned long get_next_dir(const char* path) {  //get the next directory #
#ifdef USE_SD
  uint8_t buf[4];
  File Datafile = SD.open(path);
  Datafile.read(buf, 4);  //dumps the 4 first bytes
  Datafile.read(buf, 4);
  Next_dir = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]));
  Datafile.close();
#endif
  return Next_dir;
}

void store_next_ID(const char* path, unsigned long Next_ID, unsigned long Next_dir) {  //store current file # and directory #
#ifdef USE_SD
  uint8_t buf[4];
  File Datafile = SD.open(path, FILE_WRITE);
  Datafile.seek(0);
  buf[3] = Next_ID >> 0;
  buf[2] = Next_ID >> 8;
  buf[1] = Next_ID >> 16;
  buf[0] = Next_ID >> 24;
  Datafile.write(buf, 4);
  buf[3] = Next_dir >> 0;
  buf[2] = Next_dir >> 8;
  buf[1] = Next_dir >> 16;
  buf[0] = Next_dir >> 24;
  Datafile.write(buf, 4);
  Datafile.close();
#endif
}

bool Get_JSON_config(const char* path) {  //I've copy paste the library examples, josn file stores the user parameters
  bool JSON_OK = 0;

#ifdef USE_SD
  if (SD.exists(path)) {
    JSON_OK = 1;
    File Datafile = SD.open(path);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, Datafile);
    RAW_recording_mode = doc["timelapserawrecordingMode"];
    for (int i = 0; i < num_timelapses; i++) {
      timelapse_list[i] = doc["timelapseDelay"][i];
    }
    PRETTYBORDER_mode = doc["prettyborderMode"];
    //HDR_mode = doc["HDRMode"];
    NIGHT_mode = doc["nightMode"];
    motion_detection_threshold = doc["motiondetectionThreshold"];
    for (int i = 0; i < num_HDR_images; i++) {
      exposure_list[i] = doc["hdrExposures"][i];
    }
    for (int i = 0; i < 48; i++) {
      Dithering_patterns_regular[i] = doc["ditherMatrix"][i];
    }
    GBCAMERA_mode = doc["gameboycameraMode"];
    GB_v_min = doc["lowvoltageThreshold"];
    GB_v_max = doc["highvoltageThreshold"];
    BORDER_mode = doc["enforce2DEnhancement"];
    SMOOTH_mode = doc["cancel2DEnhancement"];
    for (int i = 0; i < 48; i++) {
      Dithering_patterns_low[i] = doc["lowditherMatrix"][i];
    }
    for (int i = 0; i < 48; i++) {
      Dithering_patterns_high[i] = doc["highditherMatrix"][i];
    }
    FIXED_EXPOSURE_mode = doc["fixedExposure"];
    FIXED_delay = doc["fixedDelay"];
    FIXED_divider = doc["fixedDivider"];
    for (int i = 0; i < 256; i++) {
      lookup_TFT_RGB565[i] = doc["lookupTableRGB565"][i];
    }
    x_box = doc["exposurexWindow"];
    y_box = doc["exposureyWindow"];
    FOCUS_mode = doc["focusPeaking"];
    FOCUS_threshold = doc["focuspeakingThreshold"];
    M64283FP = doc["M64283FPsensor"];
    SLIT_SCAN_mode = doc["slitscanMode"];
    Datafile.close();
  }
#endif

  return JSON_OK;
}

void Put_JSON_config(const char* path) {  //I've copy paste the library examples, just used at boot to cycle the border #

#ifdef USE_SD
  JsonDocument doc;
  File Datafile = SD.open(path);
  deserializeJson(doc, Datafile);
  Datafile.close();

  doc["prettyborderMode"] = PRETTYBORDER_mode;  //Update some data
  Datafile = SD.open(path, "w");
  serializeJson(doc, Datafile);  //overwrite file
  Datafile.close();
#endif
}

void dump_data_to_SD_card() {  //self explanatory

#ifdef USE_SD
  File Datafile = SD.open(storage_file_name, FILE_WRITE);
  // if the file is writable, write to it:
  if (Datafile) {
    if (PRETTYBORDER_mode == 0) {
      if ((RAW_recording_mode == 0) | ((image_TOKEN == 1) & (MOTION_sensor == 0))) {  //forbid raw recording in single shot mode
        Datafile.write(BMP_header_generic, 54);                                       //fixed header for 128*120 image
        Datafile.write(BMP_indexed_palette, 1024);                                    //indexed RGB palette
        Datafile.write(BmpData, 128 * max_line_for_recording);                        //removing last tile line
        Datafile.close();
      } else {
        Datafile.write("RAWDAT");  //Just a keyword
        Datafile.write(128);
        Datafile.write(max_line_for_recording);
        Datafile.write(camReg, 8);  //camera registers from the preceding image, close to the current one
        Datafile.write(BmpData, 128 * max_line_for_recording);
        Datafile.close();
      }
    }

    if (PRETTYBORDER_mode > 0) {
      if ((RAW_recording_mode == 0) | ((image_TOKEN == 1) & (MOTION_sensor == 0))) {  //forbid raw recording in single shot mode
        Datafile.write(BMP_header_generic, 54);                                       //fixed header for 160*144 image
        Datafile.write(BMP_indexed_palette, 1024);                                    //indexed RGB palette
        Datafile.write(BigBmpData, 160 * 144);                                        //removing last tile line
        Datafile.close();
      } else {
        Datafile.write("RAWDAT");  //Just a keyword
        Datafile.write(160);
        Datafile.write(144);
        Datafile.write(camReg, 8);  //camera registers from the preceding image, close to the current one
        Datafile.write(BigBmpData, 160 * 144);
        Datafile.close();
      }
    }
  }
#endif
  delay(25);  //allows current draw to stabilize before taking another shot
}

void Pre_allocate_bmp_header(unsigned int bitmap_width, unsigned int bitmap_height) {
  //https://web.maths.unsw.edu.au/~lafaye/CCM/video/format-bmp.htm
  //https://en.wikipedia.org/wiki/BMP_file_format
  unsigned int header_size = 54;                                                      //standard header total size
  unsigned int palette_size = 1024;                                                   //indexed RGB palette here R,G,B,0 * 256 colors
  unsigned int color_planes = 1;                                                      //must be 1
  unsigned int bits_per_pixel = 8;                                                    //Typical values are 1, 4, 8, 16, 24 and 32. Here 8 bits grayscale image
  unsigned long pixel_data_size = bitmap_width * bitmap_height * bits_per_pixel / 8;  //must be a multiple of 4, this is the size of the raw bitmap data
  unsigned long total_file_size = pixel_data_size + header_size + palette_size;       //The size of the BMP file in bytes
  unsigned long starting_pixel_data_offset = palette_size + header_size;              //offset at which pixel data are stored
  unsigned long header_intermediate_size = 40;                                        //Size of header in bytes after offset 0x0A, so header_intermediate_size + 0x0A = header_size
  unsigned long bitmap_width_pixels = bitmap_width;
  unsigned long bitmap_height_pixels = -bitmap_height;  //must be inverted to have the image NOT upside down, weird particularity of this format...
  unsigned long color_number_in_palette = 256;
  //The header field used to identify the BMP and DIB file is 0x42 0x4D in hexadecimal, same as BM in ASCII.
  BMP_header_generic[0] = 0x42;  //"B" in ASCII, BMP signature
  BMP_header_generic[1] = 0x4D;  //"M" in ASCII, BMP signatureBMP signature

  //The size of the BMP file in bytes
  BMP_header_generic[2] = total_file_size >> 0;
  BMP_header_generic[3] = total_file_size >> 8;
  BMP_header_generic[4] = total_file_size >> 16;
  BMP_header_generic[5] = total_file_size >> 24;

  //next bytes reserved, not used

  //The offset, i.e. starting address, of the byte where the bitmap image data (pixel array) can be found.
  BMP_header_generic[10] = starting_pixel_data_offset >> 0;
  BMP_header_generic[11] = starting_pixel_data_offset >> 8;
  BMP_header_generic[12] = starting_pixel_data_offset >> 16;
  BMP_header_generic[13] = starting_pixel_data_offset >> 24;

  //the size of this header, in bytes (40)
  BMP_header_generic[14] = header_intermediate_size >> 0;
  BMP_header_generic[15] = header_intermediate_size >> 8;
  BMP_header_generic[16] = header_intermediate_size >> 16;
  BMP_header_generic[17] = header_intermediate_size >> 24;

  //the bitmap width in pixels (signed integer)
  BMP_header_generic[18] = bitmap_width_pixels >> 0;
  BMP_header_generic[19] = bitmap_width_pixels >> 8;
  BMP_header_generic[20] = bitmap_width_pixels >> 16;
  BMP_header_generic[21] = bitmap_width_pixels >> 24;

  //the bitmap height in pixels (signed integer)
  BMP_header_generic[22] = bitmap_height_pixels >> 0;
  BMP_header_generic[23] = bitmap_height_pixels >> 8;
  BMP_header_generic[24] = bitmap_height_pixels >> 16;
  BMP_header_generic[25] = bitmap_height_pixels >> 24;

  //the number of color planes (must be 1)
  BMP_header_generic[26] = color_planes >> 0;
  BMP_header_generic[27] = color_planes >> 8;

  //the number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 and 32
  BMP_header_generic[28] = bits_per_pixel >> 0;
  BMP_header_generic[29] = bits_per_pixel >> 8;

  //next bytes, the compression method being used, not used

  //the image size. This is the size of the raw bitmap data; a dummy 0 can be given for BI_RGB bitmaps.
  BMP_header_generic[34] = pixel_data_size >> 0;
  BMP_header_generic[35] = pixel_data_size >> 8;
  BMP_header_generic[36] = pixel_data_size >> 16;
  BMP_header_generic[37] = pixel_data_size >> 24;

  //next bytes, horizontal resolution of the image. (pixel per metre, signed integer), not used

  //next bytes, vertical resolution of the image. (pixel per metre, signed integer), not used

  //the number of colors in the color palette, or 0 to default to 2^n
  BMP_header_generic[46] = color_number_in_palette >> 0;
  BMP_header_generic[47] = color_number_in_palette >> 8;
  BMP_header_generic[48] = color_number_in_palette >> 16;
  BMP_header_generic[49] = color_number_in_palette >> 24;

  //next bytes, the number of important colors used, or 0 when every color is important; generally ignored, not used
}

//////////////////////////////////////////////Display stuff///////////////////////////////////////////////////////////////////////////////////////////
void short_fancy_delay() {
#ifndef USE_SNEAK_MODE
  for (int i = 0; i < 10; i++) {
    gpio_put(RED, 1);
    delay(20);
    gpio_put(RED, 0);
    delay(20);
  }
#endif
}

void display_other_informations() {
#ifdef USE_TFT
  current_exposure = get_exposure(camReg);  //get the current exposure register for TFT display
#ifdef DEBUG_MODE                           ////////////////beginning of debug informations//////////////////////////////////////
  if (current_exposure > 0x0FFF) {
    sprintf(exposure_string, "REG: %X", current_exposure);  //concatenate string for display
  }
  if (current_exposure <= 0x0FFF) {
    sprintf(exposure_string, "REG: 0%X", current_exposure);  //concatenate string for display;
  }
  if (current_exposure <= 0x00FF) {
    sprintf(exposure_string, "REG: 00%X", current_exposure);  //concatenate string for display;
  }
  if (current_exposure <= 0x000F) {
    sprintf(exposure_string, "REG: 000%X", current_exposure);  //concatenate string for display;
  }

  if (currentTime_exp > 1000) {
    sprintf(exposure_string_ms, "Exposure: %d ms", currentTime_exp);  //concatenate string for display;
  }
  if (currentTime_exp <= 1000) {
    sprintf(exposure_string_ms, "Exposure: 0%d ms", currentTime_exp);  //concatenate string for display;
  }
  if (currentTime_exp <= 100) {
    sprintf(exposure_string_ms, "Exposure: 00%d ms", currentTime_exp);  //concatenate string for display;
  }
  //  if (currentTime_exp <= 10) {
  //    sprintf(exposure_string_ms, "Exposure: 000%d millis", currentTime_exp);  //concatenate string for display;
  //  }
  if ((current_exposure * 16) <= 10000) {
    sprintf(exposure_string_ms, "Exposure: %d us", current_exposure * 16);  //concatenate string for display;
  }
  if ((current_exposure * 16) <= 1000) {
    sprintf(exposure_string_ms, "Exposure: 0%d us", current_exposure * 16);  //concatenate string for display;
  }

  sprintf(multiplier_string, "Clock/%X", clock_divider);  //concatenate string for displaying night mode;
  //sprintf(error_string, "Error: +%d", int(exposure_error));

  img.setTextColor(TFT_ORANGE);
  img.setCursor(8, 18);
  img.println(exposure_string_ms);  //in ms
  img.setCursor(8, 126);
  img.println(multiplier_string);
  img.setCursor(64, 126);
  img.println(exposure_string);

  if (exposure_error >= 0) {
    sprintf(error_string, "Error: +%d", int(exposure_error));
  } else {
    sprintf(error_string, "Error: %d", int(exposure_error));
  }
  img.setTextColor(TFT_ORANGE);
  img.setCursor(8, 24);
  img.println(error_string);
#endif  ////////////////end of debug informations//////////////////////////////////////

#ifdef DEBAGAME_MODE                                          ///begining of debagame mode//////////////////////////////////////
  dark_level = (masked_pixels / (128)) * (3.3 / 255) * 1000;  //get the average of the last line of pixels (masked), convert in mV
  sprintf(mask_pixels_string, "Dark: %d mV", dark_level);     //average voltage of masked pixels (dark level) in mV, is equal to Vref + reg O + Voffset
  //The sum Vref + reg O + offset must be as close as V ref as possible by adjsuting register O (fine tuning)
  //other said, the Voffset must ideally be cancelled by reg O (the Voffset varies vary with exposure time, gain, sensor, temperature, etc.)
  //in order to ensure an independance of "image aspect" to the sensor.
  img.setCursor(2, 86);
  img.println(mask_pixels_string);

  V_ref = (camReg[7] & 0b00000111) * (1000 * 0.5);    //get register V (Vref), convert in mV
  sprintf(mask_pixels_string, "Vref: %d mV", V_ref);  //theoretical Vref
  img.setCursor(2, 94);
  img.println(mask_pixels_string);

  O_reg = (camReg[0] & 0b00011111) * 32;  //get register O (1 bit sign + 5 bits (32 steps) of 32 mV)
  if ((camReg[0] & 0b00100000) == 0x20) {
    O_reg = O_reg;  //register O positive, do nothing, max +992mV
  } else {
    O_reg = O_reg * -1;  //register O negative, max -992mV
  }
  sprintf(mask_pixels_string, "Oreg: %d mV", O_reg);
  img.setCursor(2, 102);
  img.println(mask_pixels_string);

  //pure offset = dark level - (Vref + register O), must be as close as zero as possible
  V_Offset = dark_level - (V_ref + O_reg);
  sprintf(mask_pixels_string, "Voff: %d mV", V_Offset);  //pure offset, must ideally be exactly cancelled by register O
  img.setCursor(2, 110);
  img.println(mask_pixels_string);

  //this part writes the camera registers on screen
  img.setCursor(2, 118);
  if (camReg[0] < 0x10) {
    img.print("0");
  }
  img.println(camReg[0], HEX);

  img.setCursor(18, 118);
  if (camReg[1] < 0x10) {
    img.print("0");
  }
  img.println(camReg[1], HEX);

  img.setCursor(34, 118);
  if (camReg[2] < 0x10) {
    img.print("0");
  }
  img.println(camReg[2], HEX);

  img.setCursor(50, 118);
  if (camReg[3] < 0x10) {
    img.print("0");
  }
  img.println(camReg[3], HEX);

  img.setCursor(66, 118);
  img.print("0");
  img.println(camReg[4], HEX);
  img.setCursor(82, 118);
  img.print("0");
  img.println(camReg[5], HEX);
  img.setCursor(98, 118);
  img.print("0");
  img.println(camReg[6], HEX);

  img.setCursor(114, 118);
  if (camReg[7] < 0x10) {
    img.print("0");
  }
  img.println(camReg[7], HEX);
#endif  /////////////////////end of debagame mode//////////////////////////////////////

  img.setCursor(0, 0);
  img.setTextColor(TFT_CYAN);
  if (TIMELAPSE_mode == 0) {
    if (MOTION_sensor == 0) {
      if (SLIT_SCAN_mode == 1) {
        img.println("Slit Scan mode");
        img.drawLine(64, y_min + display_offset, 64, y_max + display_offset, TFT_YELLOW);
      } else {
        img.println("Regular Camera mode");
      }
    }
    if (MOTION_sensor == 1) {
      if (RAW_recording_mode == 0) {
        img.println("Motion Sensor BMP");
      }
      if (RAW_recording_mode == 1) {
        img.println("Motion Sensor RAW");
      }
    }
  }
  if (TIMELAPSE_mode == 1) {
    if (RAW_recording_mode == 0) {
      img.println("Time Lapse Mode BMP");
    }
    if (RAW_recording_mode == 1) {
      img.println("Time Lapse Mode RAW");
    }
  }

  //this part draws the exposure area on screen, majenta default, green when exposure is locked
  if (LOCK_exposure == 1) {
    img.drawRect(0, display_offset, 128, max_line, TFT_GREEN);
    img.drawLine(x_min, y_min + display_offset, x_min + line_length, y_min + display_offset, TFT_GREEN);
    img.drawLine(x_min, y_min + display_offset, x_min, y_min + display_offset + line_length, TFT_GREEN);
    img.drawLine(x_max, y_min + display_offset, x_max - line_length, y_min + display_offset, TFT_GREEN);
    img.drawLine(x_max, y_min + display_offset, x_max, y_min + display_offset + line_length, TFT_GREEN);
    img.drawLine(x_min, y_max + display_offset, x_min + line_length, y_max + display_offset, TFT_GREEN);
    img.drawLine(x_min, y_max + display_offset, x_min, y_max + display_offset - line_length, TFT_GREEN);
    img.drawLine(x_max, y_max + display_offset, x_max - line_length, y_max + display_offset, TFT_GREEN);
    img.drawLine(x_max, y_max + display_offset, x_max, y_max + display_offset - line_length, TFT_GREEN);

    sprintf(exposure_string_ms, "Exposure: LOCKED");
  } else {
    img.drawRect(0, display_offset, 128, max_line, TFT_MAGENTA);
    img.drawLine(x_min, y_min + display_offset, x_min + line_length, y_min + display_offset, TFT_MAGENTA);
    img.drawLine(x_min, y_min + display_offset, x_min, y_min + display_offset + line_length, TFT_MAGENTA);
    img.drawLine(x_max, y_min + display_offset, x_max - line_length, y_min + display_offset, TFT_MAGENTA);
    img.drawLine(x_max, y_min + display_offset, x_max, y_min + display_offset + line_length, TFT_MAGENTA);
    img.drawLine(x_min, y_max + display_offset, x_min + line_length, y_max + display_offset, TFT_MAGENTA);
    img.drawLine(x_min, y_max + display_offset, x_min, y_max + display_offset - line_length, TFT_MAGENTA);
    img.drawLine(x_max, y_max + display_offset, x_max - line_length, y_max + display_offset, TFT_MAGENTA);
    img.drawLine(x_max, y_max + display_offset, x_max, y_max + display_offset - line_length, TFT_MAGENTA);
  }
  img.setTextColor(TFT_WHITE);
  img.setCursor(0, 136);
  img.println(storage_file_name);
  img.setTextColor(TFT_GREEN);
  img.setCursor(0, 144);
  if (recording == 0) {
    if (TIMELAPSE_mode == 1) {
      sprintf(storage_deadtime, "Delay: %d ms", TIMELAPSE_deadtime);  //concatenate string for display
      img.println(storage_deadtime);
    } else {
      if (MOTION_sensor == 0) {
        img.println("No delay");
      }
      if (MOTION_sensor == 1) {
        if ((millis() - currentTime_MOTION) > delay_MOTION) {
          img.println("TIMELAPSE->exit");
        } else {
          img.setTextColor(TFT_RED);
          img.println("Sensor pre-delay");
        }
      }
    }
  }
  if (recording == 1) {
    sprintf(remaining_deadtime, "Delay: %d ms", TIMELAPSE_deadtime - (currentTime - previousTime));  //concatenate string for display
    img.println(F(remaining_deadtime));
  }
  img.setCursor(0, 152);
  if (HDR_mode == 1) {
    img.setTextColor(TFT_RED);
    img.println("HDR mode ON");
  }
  if (HDR_mode == 0) {
    img.setTextColor(TFT_GREEN);
    img.println("HDR mode OFF");
  }

  if (GBCAMERA_mode == 1) {
    if (overshooting == 1) {
      img.setTextColor(TFT_RED);
    } else {
      img.setTextColor(TFT_ORANGE);
    }
    img.setCursor(120, 152);
    if (dithering_strategy[register_strategy] == 1) {
      img.println("H");
    } else {
      img.println("L");
    }
    img.setCursor(114, 152);
    img.println(register_strategy, DEC);
  } else {
    img.setTextColor(TFT_ORANGE);
    img.setCursor(114, 152);
    img.println("--");
  }

  if (M64283FP == 0) {
    img.setTextColor(TFT_CYAN);
    img.setCursor(102, 144);
    img.println("82FP");
  }
  if (M64283FP == 1) {
    img.setTextColor(TFT_ORANGE);
    img.setCursor(102, 144);
    img.println("83FP");
  }
#endif
}

//the void sequence is elaborated in order to have a dramatic effect
void init_sequence() {  //not 100% sure why, but screen must be initialized before the SD...

#ifdef USE_TFT
  tft.init();
  tft.setRotation(2);
  //tft.invertDisplay(1);
  img.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  img.createSprite(128, 160);         // then create the giant sprite that will be our video ram buffer

#ifdef ST7789
  tft.fillScreen(TFT_BLACK);
#endif

  img.setTextSize(1);  // characters are 8x8 pixels in size 1, practical !
  for (unsigned char x = 0; x < 128; x++) {
    for (unsigned char y = 0; y < 160; y++) {
      img.drawPixel(x, y, lookup_TFT_RGB565[splashscreen[x + y * 128]]);
    }
  }
  img.pushSprite(x_ori, y_ori);  // dump image to display
  delay(250);
  img.setTextColor(TFT_WHITE);
  img.setCursor(0, 0);
  img.println("SD card:");
  img.pushSprite(x_ori, y_ori);  // dump image to display
#endif

  //see if the card is present and can be initialized
#ifdef USE_SD
  if (SD.begin(CHIPSELECT)) {
    SDcard_READY = 1;
  } else {
    SDcard_READY = 0;
  }
#endif

#ifdef USE_TFT
  if (SDcard_READY == 1) {
    img.setTextColor(TFT_GREEN);
    img.setCursor(50, 0);
    img.println("READY");
  } else {
    img.setTextColor(TFT_RED);
    img.setCursor(50, 0);
    img.println("FAIL");
  }
  img.setTextColor(TFT_WHITE);
  img.setCursor(0, 8);
  img.println("Sensor:");
  img.pushSprite(x_ori, y_ori);  // dump image to display
#endif

  //see if the sensor is present and responds, initiate registers
  memcpy(camReg, camReg_M64282FP, 8);  // just to initiate the loop
  camReset();                          // resets the sensor
  camSetRegisters();                   // Send 8 registers to the sensor
  sensor_READY = camTestSensor();      // dumb sensor cycle
  camReset();

#ifdef USE_TFT
  if (sensor_READY == 1) {
    img.setTextColor(TFT_GREEN);
    img.setCursor(50, 8);
    img.println("READY");
  } else {
    img.setTextColor(TFT_RED);
    img.setCursor(50, 8);
    img.println("FAIL");
  }
  img.setTextColor(TFT_WHITE);
  img.setCursor(0, 16);
  img.println("Config:");
  img.pushSprite(x_ori, y_ori);  //dump image to display
#endif

#ifdef USE_SD
  JSON_ready = Get_JSON_config("/config.json");  //get configuration data if a file exists
  if (PRETTYBORDER_mode > max_border) {
    PRETTYBORDER_mode = 0;
  }
#endif

#ifdef USE_TFT
  if (JSON_ready == 1) {
    img.setTextColor(TFT_GREEN);
    img.setCursor(50, 16);
    img.println("READY");
  } else {
    img.setTextColor(TFT_ORANGE);
    img.setCursor(50, 16);
    img.println("ERROR");
  }

  img.setTextColor(TFT_CYAN);
  img.setCursor(0, 24);

  //reboot when this text appears to cycle between borders
  switch (PRETTYBORDER_mode) {
    case 0:
      img.println("No border mode");
      break;
    case 1:
      img.println("Border: Custom");
      break;
    case 2:
      img.println("Border: Int. GBCam");
      break;
    case 3:
      img.println("Border: Jpn. GBCam");
      break;
    case 4:
      img.println("Border: Diag. GBCam");
      break;
    case 5:
      img.println("Border: Wave. GBCam");
      break;
    case 6:
      img.println("Border: TV. GBCam");
      break;
    case 7:
      img.println("Border: M64283FP");
      break;
    default:
      {
        img.setTextColor(TFT_RED);
        img.println("BORDER ERROR");  //this must never happen
      }
  }

  if ((SDcard_READY == 0) | (sensor_READY == 0)) {
    img.setTextColor(TFT_RED);
    img.setCursor(0, 32);
    img.println("CHECK CONNECTIONS");
  } else {
    img.setTextColor(TFT_GREEN);
    img.setCursor(0, 32);
    img.println("Preset exposure...");
  }
  img.pushSprite(x_ori, y_ori);  //dump image to display
#endif

#ifdef USE_SD
  if ((SDcard_READY == 0) | (sensor_READY == 0)) {  //get stuck here if any problem to avoid further board damage

#ifdef USE_TFT
    for (unsigned char x = 0; x < 128; x++) {
      for (unsigned char y = 0; y < 112; y++) {
        img.drawPixel(x, y + 44, lookup_TFT_RGB565[crashscreen[x + y * 128]]);  //https://tcrf.net/Proto:Game_Boy_Camera
      }
    }
    img.pushSprite(x_ori, y_ori);  //dump image to display
#endif

    while (1) {

#ifndef USE_SNEAK_MODE
      gpio_put(RED, 1);
#endif

      delay(250);
      gpio_put(RED, 0);
      delay(250);
    }
  }
#endif
}

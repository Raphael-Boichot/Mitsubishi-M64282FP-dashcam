//By Raphaël BOICHOT, made around 2023-01-27, Beware, I'm not a C developper at all so it won't be a pretty code !
//this is an autonomous recorder for the Mitsubishi M64282FP artificial retina of the Game Boy Camera
//from a code of Laurent Saint-Marcel (lstmarcel@yahoo.fr) written in 2005/07/05
//stole some code for Rafael Zenaro NeoGB printer: https://github.com/zenaro147/NeoGB-Printer
//version for Arduino here (requires a computer): https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor
//Made to be compiled on the Arduino IDE, using these libraries:
//https://github.com/earlephilhower/arduino-pico core for pi-pico
//https://github.com/Bodmer/TFT_eSPI library for TFT display
//https://arduinojson.org/ for config.json file support

//CamData[128 * 128] contains the raw signal from sensor in 8 bits
//HDRData[128 * 128] contains an average of raw signal data from sensor for dithering in 32 bits
//lookup_serial[CamData[...]] is the sensor data with autocontrast in 8 bits
//lookup_TFT_RGB565[lookup_serial[CamData[...]]] is the sensor data with autocontrast in 16 bits RGB565
//BayerData[128 * 128] contains the sensor data with dithering AND autocontrast in 2 bits
//lookup_TFT_RGB565[BayerData[...]] contains the sensor data with dithering AND autocontrast in 16 bits RGB565
//and so on...

#include "ArduinoJson.h"
#include "pico/stdlib.h"
#include "hardware/adc.h" //the GPIO commands are here
#include "big_data.h"
#include "config.h"
#include "splash.h"
#include "prettyborder.h"

#ifdef  USE_SD
#include <SPI.h>  //for SD
#include <SD.h>  //for SD
#endif

// tft directly addresses the display, im is a memory buffer for sprites
// Here I create and update a giant 128*16 sprite in memory that I push to the screen when necessary, which is ultra fast because it uses DMA
#ifdef  USE_TFT
#include <TFT_eSPI.h> // Include the graphics library (this includes the sprite functions)
TFT_eSPI    tft = TFT_eSPI();         // Create object "tft"
TFT_eSprite img = TFT_eSprite(&tft);  // Create Sprite object "img" with pointer to "tft" object. The pointer is used by pushSprite() to push it onto the TFT
#endif

//the ADC resolution is 0.8 mV (3.3/2^12, 12 bits) cut to 12.9 mV (8 bits), registers are close of those from the Game Boy Camera in mid light
//With these registers, the output voltage is between 0.58 and 3.04 volts (on 3.3 volts), this is the best I can do.
unsigned char camReg[8] = {0b10011111, 0b11101000, 0b00000001, 0b00000000, 0b00000001, 0b000000000, 0b00000001, 0b00000011}; //registers
unsigned char v_min = 45; //minimal voltage returned by the sensor in 8 bits DEC (0.58 volts)
unsigned char v_max = 236;//maximal voltage returned by the sensor in 8 bits DEC (3.04 volts)
unsigned char lookup_serial[256];//autocontrast table generated in setup() from v_min and v_max
unsigned char CamData[128 * 128];// sensor data in 8 bits per pixel
unsigned char BmpData[128 * 128];// sensor data with autocontrast ready to be merged with BMP header
unsigned char BigBmpData[160 * 144];// sensor data with autocontrast and pretty border ready to be merged with BMP header
unsigned int HDRData[128 * 128];// cumulative data for HDR imaging -1EV, +1EV + 2xOEV, 4 images in total
unsigned char Bayer_matW_LG[4 * 4];//Bayer matrix to apply dithering for each image pixel white to light gray
unsigned char Bayer_matLG_DG[4 * 4];//Bayer matrix to apply dithering for each image pixel light gray to dark gray
unsigned char Bayer_matDG_B[4 * 4];//Bayer matrix to apply dithering for each image pixel dark gray to dark
unsigned char BayerData[128 * 128];// dithered image data
unsigned int cycles = 7; //time delay in processor cycles, to fit with the 1MHz advised clock cycle for the sensor (set with a datalogger, do not touch !)
unsigned int clock_divider = 1; //time delay in processor cycles to cheat the exposure of the sensor
unsigned int debouncing_delay = 500; //debouncing delay for pushbuttons
unsigned long currentTime = 0;
unsigned long previousTime = 0;
unsigned long currentTime_exp = 0;
unsigned long previousTime_exp = 0;
unsigned long Next_ID, Next_dir;//for directories and filenames
unsigned long file_number;
unsigned int current_exposure, new_exposure;
unsigned int files_on_folder = 0;
unsigned int max_files_per_folder = 1024;
bool image_TOKEN = 0; //reserved for CAMERA mode
bool recording = 0;//0 = idle mode, 1 = recording mode
bool sensor_READY = 0;//reserved, for bug on sensor
bool SDcard_READY = 0;//reserved, for bug on SD
bool JSON_ready = 0; //reserved, for bug on config.txt
bool LOCK_exposure = 0; //reserved, for locking exposure
char storage_file_name[20], storage_file_dir[20], storage_deadtime[20], exposure_string[20];
char multiplier_string[20], error_string[20], remaining_deadtime[20], exposure_string_ms[20], files_on_folder_string[20];
char num_HDR_images = sizeof(exposure_list) / sizeof( double );//get the HDR or multi-exposure list size

//////////////////////////////////////////////Setup, core 0/////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  //interface input/output
  gpio_init(PUSH);      gpio_set_dir(PUSH, GPIO_IN);//action button to record
  gpio_init(TLC);       gpio_set_dir(TLC, GPIO_IN);//timelapse<->regular camera mode
  gpio_init(DITHER);    gpio_set_dir(DITHER, GPIO_IN);//dithering with Bayer matrix
  gpio_init(HDR);       gpio_set_dir(HDR, GPIO_IN);//HDR mode
  gpio_init(LOCK);      gpio_set_dir(LOCK, GPIO_IN);//LOCK exposure
  gpio_init(LED);       gpio_set_dir(LED, GPIO_OUT);//green LED
  gpio_init(RED);       gpio_set_dir(RED, GPIO_OUT);//red LED
  gpio_init(INT);       gpio_set_dir(INT, GPIO_OUT);//internal LED

  //sensor input/output
  gpio_init(READ);      gpio_set_dir(READ, GPIO_IN);
  gpio_init(CLOCK);     gpio_set_dir(CLOCK, GPIO_OUT);
  gpio_init(RESET);     gpio_set_dir(RESET, GPIO_OUT);
  gpio_init(LOAD);      gpio_set_dir(LOAD, GPIO_OUT);
  gpio_init(SIN);       gpio_set_dir(SIN, GPIO_OUT);
  gpio_init(START);     gpio_set_dir(START, GPIO_OUT);

  //analog stuff
  adc_gpio_init(VOUT);  adc_select_input(0);//there are several ADC channels to choose from
  adc_init();//mandatory, without it stuck the camera

#ifdef USE_OVERCLOCKING
  cycles = 15;//about twice as fast as the regular 133 MHz
  set_sys_clock_khz(250000, true);//about twice as fast as the regular 133 MHz
#endif

#ifdef DEBUG_MODE // serial is optional, only needed for debugging or interfacing with third party soft via USB cable
  Serial.begin(2000000);
#endif

  init_sequence();//Boot screen get stuck here with red flashing LED if any problem with SD or sensor to avoid further board damage
  //now if code arrives at this point, this means that sensor and SD card are connected correctly in normal use
  sprintf(storage_deadtime, "Delay: %d ms", TIMELAPSE_deadtime); //concatenate string for display

#ifdef  USE_SD
  ID_file_creator("/Dashcam_storage.bin");//create a file on SD card that stores a unique file ID from 1 to 2^32 - 1 (in fact 1 to 99999)
  Next_ID = get_next_ID("/Dashcam_storage.bin");//get the file number on SD card
  Next_dir = get_next_dir("/Dashcam_storage.bin");//get the folder number on SD card
#endif

  pre_allocate_lookup_tables(lookup_serial, v_min, v_max); //pre allocate tables for TFT and serial output auto contrast
  pre_allocate_Bayer_tables();// just reordering the Game Boy Camera dithering registers into 3 square matrices
  pre_allocate_image_with_pretty_borders();//pre allocate bmp data for image with borders
  if (BORDER_mode == 1) camReg[1] = 0b11101000;//With 2D border enhancement
  if (BORDER_mode == 0) camReg[1] = 0b00001000;//Without 2D border enhancement (very soft image, better for nightmode)

  if (FIXED_EXPOSURE_mode == 0) { //skip if fixed exposure
    // presets the exposure time before displaying to avoid unpleasing result, maybe be slow in the dark
    for (int i = 1; i < 10; i++) {
      take_a_picture();
      new_exposure = auto_exposure(camReg, CamData, v_min, v_max);// self explanatory
      push_exposure(camReg, new_exposure, 1); //update exposure registers C2-C3
    }
  }
}


//////////////////////////////////////////////Main loop, core 0///////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
  currentTime = millis();
  take_a_picture(); //data in memory for the moment, one frame
  new_exposure = auto_exposure(camReg, CamData, v_min, v_max); // self explanatory

  if (FIXED_EXPOSURE_mode == 1) {
    new_exposure = FIXED_delay;
    clock_divider = FIXED_divider;
  }
  if (LOCK_exposure == 0) {
    push_exposure(camReg, new_exposure, 1); //update exposure registers C2-C3
  }

  /////////////////////////////////////////
  if (gpio_get(LOCK) == 1) {
    LOCK_exposure = !LOCK_exposure;//self explanatory
    short_fancy_delay();
  }
  gpio_put(INT, LOCK_exposure);

  if (gpio_get(HDR) == 1) {
    HDR_mode = !HDR_mode;//self explanatory
    short_fancy_delay();
  }
  if (gpio_get(DITHER) == 1) {
    DITHER_mode = !DITHER_mode;//self explanatory
    short_fancy_delay();
  }
  if ((gpio_get(TLC) == 1) & (recording == 0) & (image_TOKEN == 0)) {// Change regular camera<->timelapse mode, but only when NOT recording
    TIMELAPSE_mode = !TIMELAPSE_mode;//self explanatory
    short_fancy_delay();
  }
  ////////////////////////////////////////

  if (DITHER_mode == 1) {
    Dither_image(CamData, BayerData);
  }

#ifdef USE_SERIAL
  dump_data_to_serial(CamData);//dump raw data to serial in ASCII for debugging - you can use the Matlab code ArduiCam_Matlab.m into the repo to probe the serial and plot images
#endif

#ifdef  USE_TFT
  img.fillSprite(TFT_BLACK);// prepare the image in ram
  for (int16_t x = 1; x < 128 ; x++) {
    for (int16_t y = 0; y < 120; y++) {
      if (DITHER_mode == 1) {
        img.drawPixel(x, y + 16, lookup_TFT_RGB565[BayerData[x + y * 128]]);//BayerData includes auto-contrast
      }
      else {
        img.drawPixel(x, y + 16, lookup_TFT_RGB565[lookup_serial[CamData[x + y * 128]]]);//lookup_serial is autocontrast
      }
    }
  }
#endif

  if ((recording == 0) & (image_TOKEN == 0)) { //just put informations to the display
#ifdef  USE_TFT
    img.setTextColor(TFT_GREEN);
    img.setCursor(0, 8);
    img.println("Display Mode");
    display_other_informations();
    img.pushSprite(0, 0);// dump image to display
#endif
  }

  if ((recording == 1) & (image_TOKEN == 0)) { // prepare for recording in timelapse mode
#ifdef  USE_TFT
    img.setTextColor(TFT_RED);
    img.setCursor(0, 8);
    img.println("Recording...");
    sprintf(files_on_folder_string, "%X/%X", files_on_folder, max_files_per_folder);
    img.setCursor(84, 8);
    img.println(files_on_folder_string);
    display_other_informations();
    img.pushSprite(0, 0);// dump image to display
#endif
    if ((currentTime - previousTime) > TIMELAPSE_deadtime) {
      recording_loop();// Wait for deadtime set in config.txt
    }
    if (TIMELAPSE_deadtime > 10000) {
      sleep_ms(1000); //for timelapses with long deadtimes, no need to constantly spam the sensor for autoexposure
    }
  }//end of recording loop for timelapse

  if ((image_TOKEN == 1) & (recording == 0)) { // prepare for recording one shot
#ifdef  USE_TFT
    img.setTextColor(TFT_RED);
    img.setCursor(0, 8);
    img.println("Picture taken !");
    display_other_informations();
    img.pushSprite(0, 0);// dump image to display
#endif
    recording_loop();// Wait for deadtime set in config.txt
    image_TOKEN = 0;
#ifdef  USE_SD
    store_next_ID("/Dashcam_storage.bin", Next_ID, Next_dir);//store last known file/directory# to SD card
#endif
    delay(250);//long enough for debouncing, fast enough for a decent burst mode
  }//end of recording loop for regular camera mode

  if ((TIMELAPSE_mode == 0) && (gpio_get(PUSH) == 1)) { //camera mode acts like if user requires just one picture
#ifdef  USE_SD
    Next_ID = get_next_ID("/Dashcam_storage.bin");//get the file number on SD card
    Next_dir = get_next_dir("/Dashcam_storage.bin");//get the folder number on SD card, just to store it in memory and rewrite it at the end
#endif
    image_TOKEN = 1;
  }

  if (TIMELAPSE_mode == 1)
  {
    if ((gpio_get(PUSH) == 1) && (recording == 1)) { // we want to stop recording
      if (MOVIEMAKER_mode == 0) Next_dir++; // update next directory except in moviemaker mode

#ifdef  USE_SD
      store_next_ID("/Dashcam_storage.bin", Next_ID, Next_dir);//store last known file/directory# to SD card
#endif

      recording = 0;
      files_on_folder = 0;
      short_fancy_delay();
    }
    if ((gpio_get(PUSH) == 1) && (recording == 0)) { // we want to record: get file/directory#

      if (MOVIEMAKER_mode == 0) {
#ifdef  USE_SD
        Next_dir = get_next_dir("/Dashcam_storage.bin");//get the folder number on SD card
        sprintf(storage_file_dir, "/%06d/", Next_dir);//update next directory
        SD.mkdir(storage_file_dir);//create next directory
#endif
      }
#ifdef  USE_SD
      Next_ID = get_next_ID("/Dashcam_storage.bin");//get the file number on SD card
#endif
      if (MOVIEMAKER_mode == 1) {
        Next_ID++;
        sprintf(storage_file_name, "/Raw_data/%07d.raw", Next_ID); //update filename
      }
      recording = 1;
      previousTime = currentTime;
      short_fancy_delay();
    }
  }
}


//////////////////////////////////////////////Sensor stuff///////////////////////////////////////////////////////////////////////////////////////////

void take_a_picture() {
  camReset();// resets the sensor
  camSetRegisters();// Send 8 registers to the sensor
  camReadPicture(CamData); // get pixels, dump them in CamData
  camReset();// probably not usefull but who knows...
}

int auto_exposure(unsigned char camReg[8], unsigned char CamData[128 * 128], unsigned char v_min, unsigned char v_max) {
  double exp_regs, new_regs, error, mean_value;
  unsigned int setpoint = (v_max + v_min) >> 1;
  unsigned int accumulator = 0;
  unsigned char pixel;
  unsigned char max_line = 120; //last 5-6 rows of pixels contains dark pixel value and various artifacts, so I remove 8 to have a full tile line
  exp_regs = camReg[2] * 256 + camReg[3];// I know, it's a shame to use a double here but we have plenty of ram
  for (int i = 0; i < 128 * max_line; i++) {
    pixel = CamData[i];
    accumulator = accumulator + pixel;// accumulate the mean gray level, but only from line 0 to 120 as bottom of image have artifacts
  }
  mean_value = accumulator / (128 * max_line);
  error = setpoint - mean_value; // so in case of deviation, registers 2 and 3 are corrected
  // this part is very similar to what a Game Boy Camera does, except that it does the job with only bitshift operators and in more steps.
  // Here we can use 32 bits variables for ease of programming.
  // the bigger the error is, the bigger the correction on exposure is.
  new_regs = exp_regs;
  if (error > 80)                     new_regs = exp_regs * 2;
  if (error < -80)                    new_regs = exp_regs / 2;
  if ((error <= 80) && (error >= 30))    new_regs = exp_regs * 1.3;
  if ((error >= -80) && (error <= -30))  new_regs = exp_regs / 1.3;
  if ((error <= 30) && (error >= 10))     new_regs = exp_regs * 1.03;
  if ((error >= -30) && (error <= -10))   new_regs = exp_regs / 1.03;
  if ((error <= 10) && (error >= 4))   new_regs = exp_regs + 1;//this level is critical to avoid flickering in full sun, 3-4 is nice
  if ((error >= -10) && (error <= -4))  new_regs = exp_regs - 1;//this level is critical to avoid flickering in full sun,  3-4 is nice
  sprintf(error_string, "Error: %d", int(error)); //concatenate string for display;
  return int(new_regs);
}

void push_exposure(unsigned char camReg[8], unsigned int current_exposure, double factor) {
  double new_regs;
  new_regs = current_exposure * factor;
  if (new_regs < 0x0030) {//minimum of the sensor for these registers, below there are verticals artifacts, see sensor documentation for details
    new_regs = 0x0030;//minimum of the sensor for these registers, below there are verticals artifacts, see sensor documentation for details
  }
  if (new_regs > 0xFFFF) {//maximum of the sensor, about 1 second
    if (NIGHT_mode == 1) {
      clock_divider = clock_divider + 1 ;
    }
    new_regs = 0xFFFF;
  }

  if (NIGHT_mode == 1) {
    if (current_exposure < 0x1000) {
      clock_divider = 1 ;//Normal situation is to be always 1, so that clock is about 1MHz
    }
  }

  camReg[2] = int(new_regs / 256);//Janky, I know...
  camReg[3] = int(new_regs - camReg[2] * 256);//Janky, I know...
}

unsigned int get_exposure(unsigned char camReg[8]) {
  double exp_regs;
  exp_regs = camReg[2] * 256 + camReg[3];//
  return exp_regs;
}

void camDelay()// Allow a lag in processor cycles to maintain signals long enough
{
  for (int i = 0; i < cycles; i++) NOP;
}

void camSpecialDelay()// Allow an extra lag in processor cycles during exposure to allow night mode
{
  for (int i = 0; i < cycles * clock_divider; i++) NOP;
}

void camInit()// Initialise the IO ports for the camera
{
  gpio_put(CLOCK, 0);
  gpio_put(RESET, 1);
  gpio_put(LOAD, 0);
  gpio_put(START, 0);
  gpio_put(SIN, 0);
}

void camReset()// Sends a RESET pulse to sensor
{
  gpio_put(CLOCK, 1);
  camDelay();
  gpio_put(CLOCK, 0);
  camDelay();
  gpio_put(RESET, 0);
  camDelay();
  gpio_put(CLOCK, 1);
  camDelay();
  gpio_put(RESET, 1);
  camDelay();
}

void camSetRegisters(void)// Sets the sensor 8 registers
{
  for (int reg = 0; reg < 8; ++reg) {
    camSetReg(reg, camReg[reg]);
  }
}

void camSetReg(unsigned char regaddr, unsigned char regval)// Sets one of the 8 8-bit registers in the sensor, from 0 to 7, in this order
{ //GB camera uses another order but sensor do not mind
  unsigned char bitmask;
  for (bitmask = 0x4; bitmask >= 0x1; bitmask >>= 1) {// Write 3-bit address.
    gpio_put(CLOCK, 0);
    camDelay();
    gpio_put(LOAD, 0);// ensure load bit is cleared from previous call
    if (regaddr & bitmask) {
      gpio_put(SIN, 1);// Set the SIN bit
    }
    else {
      gpio_put(SIN, 0);
    }
    camDelay();
    gpio_put(CLOCK, 1);
    camDelay();
    gpio_put(SIN, 0);// set the SIN bit low
    camDelay();
  }
  for (bitmask = 128; bitmask >= 1; bitmask >>= 1) {// Write the 8-bits register
    gpio_put(CLOCK, 0);
    camDelay();
    if (regval & bitmask) {
      gpio_put(SIN, 1);// set the SIN bit
    }
    else {
      gpio_put(SIN, 0);
    }
    camDelay();
    if (bitmask == 1) {
      gpio_put(LOAD, 1);// Assert load at rising edge of CLOCK
    }
    gpio_put(CLOCK, 1);
    camDelay();
    gpio_put(SIN, 0);
    camDelay();
  }
}

void camReadPicture(unsigned char CamData[128 * 128]) // Take a picture, read it and send it through the serial port.
{
  unsigned int pixel;       // Buffer for pixel read in
  int x, y;
  int subcounter = 0;
  // Camera START sequence
  gpio_put(CLOCK, 0);
  camDelay();// ensure load bit is cleared from previous call
  gpio_put(LOAD, 0);
  gpio_put(START, 1);// START rises before CLOCK
  camDelay();
  gpio_put(CLOCK, 1);
  camDelay();
  gpio_put(START, 0);// START valid on rising edge of CLOCK, so can drop now
  camDelay();
  gpio_put(CLOCK, 0);
  camDelay();

#ifndef  USE_SNEAK_MODE
  gpio_put(LED, 1);
#endif
  bool skip_loop = 0;
  previousTime_exp = millis();
  while (1)
  { // Wait for READ to go high, this is the loop waiting for exposure
    gpio_put(CLOCK, 1);
    camSpecialDelay();
    if (gpio_get(READ) == 1) {
      break;// READ goes high with rising CLOCK, normal ending
    }
    if (((gpio_get(PUSH) == 1) & (image_TOKEN == 0)) | (gpio_get(HDR) == 1) | (gpio_get(DITHER) == 1) | (gpio_get(TLC) == 1) | (gpio_get(LOCK) == 1)) { //any button is pushed, skip sensor stuff
      skip_loop = 1;
      camReset();
      break;  // we want to do something, skip next steps
    }
    camDelay();
    gpio_put(CLOCK, 0);
    camDelay();
  }
  currentTime_exp = millis() - previousTime_exp; //to dislay the real exposure time, not the registers
  gpio_put(LED, 0);
  if (skip_loop == 0) {//procedure not interrupted by user
    for (y = 0; y < 128; y++) {
      for (x = 0; x < 128; x++) {
        gpio_put(CLOCK, 0);
        camDelay();
        pixel = adc_read();// The ADC is 12 bits, this sacrifies the 4 least significant bits to simplify transmission
        CamData[subcounter] = pixel >> 4;//record only
        subcounter = subcounter + 1;
        camDelay();
        gpio_put(CLOCK, 1);
        camDelay();
      } // end for x
    } /* for y */

    while (gpio_get(READ) == 1) { // Go through the remaining rows
      gpio_put(CLOCK, 0);
      camDelay();
      gpio_put(CLOCK, 1);
      camDelay();
    }
    gpio_put(CLOCK, 0);
    camDelay();
  }
}

bool camTestSensor() // dummy cycle faking to take a picture, if it's not able to go through the whole cycle, the camera will stop with error message
{ // it basically checks if READ is able to change at the good moment during the sequence
  bool sensor_OK = 1;
  int x, y;
  int currentTime;
  // Camera START sequence
  gpio_put(CLOCK, 0);
  camDelay();// ensure load bit is cleared from previous call
  gpio_put(LOAD, 0);
  gpio_put(START, 1);// START rises before CLOCK
  camDelay();
  gpio_put(CLOCK, 1);
  camDelay();
  gpio_put(START, 0);// START valid on rising edge of CLOCK, so can drop now
  camDelay();
  gpio_put(CLOCK, 0);
  camDelay();
#ifndef  USE_SNEAK_MODE
  gpio_put(LED, 1);
#endif
  currentTime = millis();
  while (1) {// Wait for READ to go high
    gpio_put(CLOCK, 1);
    camDelay();
    if (gpio_get(READ) == 1) {
      break;// READ goes high with rising CLOCK, everything is OK
    }
    if ((millis() - currentTime) > 1000) {
      sensor_OK = 0;
      break;//the sensor does not respond after 1 second = not connected
    }
    camDelay();
    gpio_put(CLOCK, 0);
  }
  camDelay();
  gpio_put(LED, 0);
  for (y = 0; y < 128; y++) {
    for (x = 0; x < 128; x++) {
      gpio_put(CLOCK, 0);
      camDelay();
      //adc_read();// doing nothing, just performing a loop
      camDelay();
      gpio_put(CLOCK, 1);
      camDelay();
    } // end for x
  } /* for y */
  currentTime = millis();
  while (gpio_get(READ) == 1) { // Go through the remaining rows, but READ can stay high due to level shifter design
    gpio_put(CLOCK, 0);
    camDelay();
    gpio_put(CLOCK, 1);
    camDelay();
    if ((millis() - currentTime) > 1000) {
      sensor_OK = 0;
      break;//the sensor does not respond after 1 seconds = not connected
    }
  }
  gpio_put(CLOCK, 0);
  camDelay();
  return sensor_OK;
}


//////////////////////////////////////////////Output stuff///////////////////////////////////////////////////////////////////////////////////////////
void recording_loop()
{
  if ((MOVIEMAKER_mode == 0) | (image_TOKEN == 1)) {
    Next_ID++; // update the file number, but not in movie maker mode
  }
  previousTime = currentTime;
  if (TIMELAPSE_mode == 1) {
    if (MOVIEMAKER_mode == 0) {
      sprintf(storage_file_name, "/%06d/%07d.bmp", Next_dir, Next_ID); //update filename
    }
  }
  if (TIMELAPSE_mode == 0) {
    sprintf(storage_file_name, "/Camera/%07d.bmp", Next_ID); //update filename
  }

  if (HDR_mode == 1) {//default is 8 pictures, beware of modifying the code in case of change

#ifndef  USE_SNEAK_MODE
    gpio_put(RED, 1);
#endif

    memset(HDRData, 0, sizeof(HDRData));//clean the HDR data array
    current_exposure = get_exposure(camReg);//store the current exposure register for later
    for (int i = 0; i < num_HDR_images; i++) {
      push_exposure(camReg, current_exposure, exposure_list[i]);//vary the exposure
      take_a_picture();
      for (int i = 0; i < 128 * 128; i++) {
        HDRData[i] = HDRData[i] + CamData[i]; //sum data
      }
    }
    //now time to average all this shit
    for (int i = 0; i < 128 * 128; i++) {
      CamData[i] = HDRData[i] / num_HDR_images; //do the average
    }
    push_exposure(camReg, current_exposure, 1); //rewrite the old register stored before
  }

  if (DITHER_mode == 1) {
    Dither_image(CamData, BayerData);
    for (int i = 0; i < 128 * 128; i++) {
      BmpData[i] = BayerData[i];//to get data with dithering (dithering includes auto-contrast)
    }
  }
  else
  {
    for (int i = 0; i < 128 * 128; i++) {
      BmpData[i] = lookup_serial[CamData[i]];//to get data with autocontrast
    }
  }

  if (PRETTYBORDER_mode == 1) {
    make_image_with_pretty_borders();
  }

#ifndef  USE_SNEAK_MODE
  gpio_put(RED, 1);
#endif

  dump_data_to_SD_card();//////////////////////cannot move this to core 1 without bug, set aside for the moment

#ifdef  USE_SD
  if (TIMELAPSE_mode == 1)
  {
    files_on_folder++;
    if (files_on_folder == max_files_per_folder) {//because up to 1000 files per folder stalling or errors in writing can happens
      files_on_folder = 0;
      store_next_ID("/Dashcam_storage.bin", Next_ID, Next_dir);//in case of crash...
      if (MOVIEMAKER_mode == 0) {
        Next_dir++;
      }
    }
  }
#endif

#ifndef  USE_SNEAK_MODE
  gpio_put(RED, 0);
#endif
}


void pre_allocate_lookup_tables(unsigned char lookup_serial[256], unsigned char v_min, unsigned char v_max) {
  double gamma_pixel;
  for (int i = 0; i < 256; i++) {//building the autocontrat table lookup_serial
    if (i < v_min) {
      lookup_serial[i] = 0x00;
    }
    if ((i >= v_min) && (i <= v_max)) {
      gamma_pixel = ((i - double(v_min)) / (double(v_max) - double(v_min))) * 255;
      lookup_serial[i] = int(gamma_pixel);
    }
    if (i > v_max) {
      lookup_serial[i] = 0xFF;
    }
  }
}

void pre_allocate_Bayer_tables()
{
  // this reorganizes the thresholding matrices from Game Boy Camera registers to "Bayer like" matrices
  int counter = 0;
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      Bayer_matDG_B[x + 4 * y] = Dithering_patterns [counter];
      counter = counter + 1;
      Bayer_matLG_DG[x + 4 * y] = Dithering_patterns [counter];
      counter = counter + 1;
      Bayer_matW_LG[x + 4 * y] = Dithering_patterns [counter];
      counter = counter + 1;
    }
  }
}

void Dither_image(unsigned char CamData[128 * 128], unsigned char BayerData[128 * 128])
{ //dithering algorithm
  char pixel, pixel_out;
  int counter = 0;
  pixel_out = 0;
  for (int y = 0; y < 128; y++) {
    for (int x = 0; x < 128; x++) {
      //pixel = CamData[counter];//non auto_contrasted values, may range between 0 and 255
      pixel = lookup_serial[CamData[counter]];//auto_contrasted values, may range between 0 and 255
      pixel_out = Dithering_palette[3];
      if (pixel < Bayer_matDG_B[(x & 3) + 4 * (y & 3)]) {
        pixel_out = Dithering_palette[0];
      }
      if ((pixel >= Bayer_matDG_B[(x & 3) + 4 * (y & 3)]) & (pixel < Bayer_matLG_DG[(x & 3) + 4 * (y & 3)])) {
        pixel_out = Dithering_palette[1];
      }
      if ((pixel >= Bayer_matLG_DG[(x & 3) + 4 * (y & 3)]) & (pixel < Bayer_matW_LG[(x & 3) + 4 * (y & 3)])) {
        pixel_out = Dithering_palette[2];
      }
      BayerData[counter] = pixel_out;
      counter = counter + 1;
    }
  }
}

void make_image_with_pretty_borders()
{
  int image_counter = 8 * 128; //remove the 8 first pixel line like a Game Boy Camera
  int pretty_image_counter = 16 * 160; //beginning of first pixel to fill
  for (int line = 0; line < 112; line++) {//line counter
    pretty_image_counter = pretty_image_counter + 16;
    for (int x = 0; x < 128; x++) {//column counter
      BigBmpData[pretty_image_counter] = BmpData[image_counter];
      pretty_image_counter++;
      image_counter++;
    }
    pretty_image_counter = pretty_image_counter + 16;
  }
}

void pre_allocate_image_with_pretty_borders()
{
  memset(BigBmpData, 0, sizeof(BigBmpData));//clean the BigBmpData data array
  int counter = 0;
  for (int y = 0; y < 144; y++) {
    for (int x = 0; x < 160; x++) {
      BigBmpData[counter] = Dithering_palette[prettyborder[counter]];//ensures that the palette matches with the dithering palette in any case
      counter = counter + 1;
    }
  }
}

void dump_data_to_serial(unsigned char CamData[128 * 128]) {
  char pixel;
  for (int i = 0; i < 128 * 128; i++) {
    pixel = lookup_serial[CamData[i]];//to get data with autocontrast
    //pixel = CamData[i]; //to get data without autocontrast
    if (pixel <= 0x0F) {
      Serial.print('0');
    }
    Serial.print(pixel, HEX);
    Serial.print(" ");
  }
  Serial.println("");
}

//////////////////////////////////////////////SD stuff///////////////////////////////////////////////////////////////////////////////////////////

void ID_file_creator(const char * path) {
#ifdef  USE_SD
  uint8_t buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  if (!SD.exists(path)) {
    File file = SD.open(path, FILE_WRITE);
    //start from a fresh install on SD
    file.write(buf, 8);
    file.close();
  }
#endif
}

unsigned long get_next_ID(const char * path) {
#ifdef  USE_SD
  uint8_t buf[4];
  File file = SD.open(path);
  file.read(buf, 4);
  Next_ID = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]));
  file.close();
#endif
  return Next_ID;
}

unsigned long get_next_dir(const char * path)
{
#ifdef  USE_SD
  uint8_t buf[4];
  File file = SD.open(path);
  file.read(buf, 4);//dumps the 4 first bytes
  file.read(buf, 4);
  Next_dir = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]));
  file.close();
#endif
  return Next_dir;
}

void store_next_ID(const char * path, unsigned long Next_ID, unsigned long Next_dir) {
#ifdef  USE_SD
  uint8_t buf[4];
  File file = SD.open(path, FILE_WRITE);
  file.seek(0);
  buf[3] = Next_ID >>  0;
  buf[2] = Next_ID >>  8;
  buf[1] = Next_ID >> 16;
  buf[0] = Next_ID >> 24;
  file.write(buf, 4);
  buf[3] = Next_dir >>  0;
  buf[2] = Next_dir >>  8;
  buf[1] = Next_dir >> 16;
  buf[0] = Next_dir >> 24;
  file.write(buf, 4);
  file.close();
#endif
}

bool Get_JSON_config(const char * path) {//I've copy paste the library examples
  // Open file for reading
  bool JSON_OK = 0;
#ifdef  USE_SD
  if (SD.exists(path)) {
    JSON_OK = 1;
    File file = SD.open(path);
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, file);
    TIMELAPSE_mode = doc["timelapseMode"];
    MOVIEMAKER_mode = doc["timelapserawrecordingMode"];
    TIMELAPSE_deadtime = doc["timelapseDelay"];
    PRETTYBORDER_mode = doc["prettyborderMode"];
    NIGHT_mode = doc["nightMode"];
    BORDER_mode = doc["2dEnhancement"];
    HDR_mode = doc["hdrMode"];
    for (int i = 0; i < num_HDR_images; i++) {
      exposure_list[i] = doc["hdrExposures"][i];
    }
    DITHER_mode = doc["ditheringMode"];
    for (int i = 0; i < 48; i++) {
      Dithering_patterns [i] = doc["ditherMatrix"][i];
    }
    FIXED_EXPOSURE_mode = doc["fixedExposure"];
    FIXED_delay = doc["fixedDelay"];
    FIXED_divider = doc["fixedDivider"];
    file.close();
  }
#endif
  return JSON_OK;
}

void dump_data_to_SD_card()
{
#ifdef  USE_SD
  File dataFile = SD.open(storage_file_name, FILE_WRITE);
  // if the file is writable, write to it:
  if (dataFile) {
    if (PRETTYBORDER_mode == 0)
    {
      if ((MOVIEMAKER_mode == 0) | (image_TOKEN == 1)) { //forbid raw recording in single shot mode
        dataFile.write(BMP_header, 1078);//fixed header for 128*120 image
        dataFile.write(BmpData, 128 * 120); //removing last tile line
        dataFile.close();
      }

      if ((MOVIEMAKER_mode == 1) & (image_TOKEN == 0)) { //forbid raw recording in single shot mode
        dataFile.write("RAW_8BIT_128x120");//Just a marker
        dataFile.write("REGISTER");//Just a marker
        dataFile.write(camReg, 8); //camera registers from the preceding image, close to the current one
        dataFile.write(BmpData, 128 * 120);
        dataFile.close();
      }
    }
    if (PRETTYBORDER_mode == 1)
    {
      if ((MOVIEMAKER_mode == 0) | (image_TOKEN == 1)) { //forbid raw recording in single shot mode
        dataFile.write(BMP_header_prettyborder, 1078);//fixed header for 160*144 image
        dataFile.write(BigBmpData, 160 * 144); //removing last tile line
        dataFile.close();
      }

      if ((MOVIEMAKER_mode == 1) & (image_TOKEN == 0)) { //forbid raw recording in single shot mode
        dataFile.write("RAW_8BIT_160x144");//Just a marker
        dataFile.write("REGISTER");//Just a marker
        dataFile.write(camReg, 8); //camera registers from the preceding image, close to the current one
        dataFile.write(BigBmpData, 160 * 144);
        dataFile.close();
      }
    }
  }
#endif
}

//////////////////////////////////////////////Display stuff///////////////////////////////////////////////////////////////////////////////////////////
void short_fancy_delay() {
#ifndef  USE_SNEAK_MODE
  for (int i = 0; i < 10; i++) {
    gpio_put(RED, 1);
    delay(20);
    gpio_put(RED, 0);
    delay(20);
  }
#endif
}

void display_other_informations() {
#ifdef  USE_TFT

  current_exposure = get_exposure(camReg);//get the current exposure register for TFT display
  if (current_exposure > 0x0FFF) {
    sprintf(exposure_string, "REG: %X", current_exposure); //concatenate string for display
  }
  if (current_exposure <= 0x0FFF) {
    sprintf(exposure_string, "REG: 0%X", current_exposure); //concatenate string for display;
  }
  if (current_exposure <= 0x00FF) {
    sprintf(exposure_string, "REG: 00%X", current_exposure); //concatenate string for display;
  }
  if (current_exposure <= 0x000F) {
    sprintf(exposure_string, "REG: 000%X", current_exposure); //concatenate string for display;
  }

  sprintf(multiplier_string, "Clock/%X", clock_divider); //concatenate string for display;

  if (currentTime_exp > 1000) {
    sprintf(exposure_string_ms, "Exposure: %d ms", currentTime_exp); //concatenate string for display;
  }
  if (currentTime_exp <= 1000) {
    sprintf(exposure_string_ms, "Exposure: 0%d ms", currentTime_exp); //concatenate string for display;
  }
  if (currentTime_exp <= 100) {
    sprintf(exposure_string_ms, "Exposure: 00%d ms", currentTime_exp); //concatenate string for display;
  }
  if (currentTime_exp <= 10) {
    sprintf(exposure_string_ms, "Exposure: 000%d ms", currentTime_exp); //concatenate string for display;
  }

  img.setCursor(0, 0);
  img.setTextColor(TFT_CYAN);
  if (TIMELAPSE_mode == 0) {
    img.println("Regular Camera mode");
  }
  if (TIMELAPSE_mode == 1) {
    if (MOVIEMAKER_mode == 0) {
      img.println("Time Lapse Mode BMP");
    }
    if (MOVIEMAKER_mode == 1) {
      img.println("Time Lapse Mode RAW");
    }
  }
  img.setTextColor(TFT_BLUE);
  img.setCursor(8, 18);
  //img.println(exposure_string);//in register value
  img.println(exposure_string_ms);//in ms
  img.setTextColor(TFT_BLUE);
  img.setCursor(64, 126);
  img.println(exposure_string);
  //img.println(error_string);
  img.setCursor(8, 126);
  img.println(multiplier_string);
  img.setTextColor(TFT_WHITE);
  img.setCursor(0, 136);
  img.println(storage_file_name);
  img.setTextColor(TFT_GREEN);
  img.setCursor(0, 144);
  if (recording == 0) {
    img.println(storage_deadtime);
  }
  if (recording == 1) {
    sprintf(remaining_deadtime, "Delay: %d ms", TIMELAPSE_deadtime - (currentTime - previousTime)); //concatenate string for display
    img.println(F(remaining_deadtime));
  }
  img.setCursor(0, 152);
  if (HDR_mode == 1) {
    img.setTextColor(TFT_RED);
    img.println("HDR ON / USE TRIPOD!");
  }
  if (HDR_mode == 0) {
    img.setTextColor(TFT_GREEN);
    img.println("HDR mode OFF");
  }
  if (BORDER_mode == 1) {
    img.drawRect(0, 16, 128, 120, TFT_MAGENTA);
  }
#endif
}

//the void sequence is made in order to have a dramatic effect
void init_sequence() {//not 100% sure why, but screen must be initialized before the SD...
#ifdef  USE_TFT
  tft.init();
  tft.setRotation(2);
  //tft.invertDisplay(1);
  img.setColorDepth(BITS_PER_PIXEL);         // Set colour depth first
  img.createSprite(128, 160);                // then create the giant sprite that will be our video ram buffer
  img.setTextSize(1);                        // characters are 8x8 pixels in size 1, practical !
  for (int16_t x = 1; x < 128 ; x++) {
    for (int16_t y = 0; y < 160; y++) {
      img.drawPixel(x, y , lookup_TFT_RGB565[splashscreen[x + y * 128]]);
    }
  }
  img.pushSprite(0, 0);// dump image to display
  delay(500);
  img.setTextColor(TFT_WHITE);
  img.setCursor(0, 0);
  img.println("SD card:");
  img.pushSprite(0, 0);// dump image to display
#endif

  //see if the card is present and can be initialized
#ifdef  USE_SD
  if (SD.begin(CHIPSELECT)) {
    SDcard_READY = 1;
  }
  else {
    SDcard_READY = 0;
  }
#endif

#ifdef  USE_TFT
  if (SDcard_READY == 1) {
    img.setTextColor(TFT_GREEN);
    img.setCursor(50, 0);
    img.println("READY");
  }
  else {
    img.setTextColor(TFT_RED);
    img.setCursor(50, 0);
    img.println("FAIL");
  }
  img.setTextColor(TFT_WHITE);
  img.setCursor(0, 8);
  img.println("Sensor:");
  img.pushSprite(0, 0);// dump image to display
#endif

  //see if the sensor is present and responds
  camReset();// resets the sensor
  camSetRegisters();// Send 8 registers to the sensor
  sensor_READY = camTestSensor(); // dumb sensor cycle
  camReset();

#ifdef  USE_TFT
  if (sensor_READY == 1) {
    img.setTextColor(TFT_GREEN);
    img.setCursor(50, 8);
    img.println("READY");
  }
  else {
    img.setTextColor(TFT_RED);
    img.setCursor(50, 8);
    img.println("FAIL");
  }
  img.setTextColor(TFT_WHITE);
  img.setCursor(0, 16);
  img.println("Config:");
  img.pushSprite(0, 0);// dump image to display
#endif

  JSON_ready = Get_JSON_config("/config.json"); //get configuration data if a file exists

#ifdef  USE_TFT
  if (JSON_ready == 1) {
    img.setTextColor(TFT_GREEN);
    img.setCursor(50, 16);
    img.println("READY");
  }
  else {
    img.setTextColor(TFT_ORANGE);
    img.setCursor(50, 16);
    img.println("NOT FOUND");
  }
  if ((SDcard_READY == 0) | (sensor_READY == 0)) {
    img.setTextColor(TFT_RED);
    img.setCursor(0, 24);
    img.println("CHECK CONNECTIONS");
  }
  else {
    img.setTextColor(TFT_GREEN);
    img.setCursor(0, 24);
    img.println("NOW BOOTING...");
  }
  img.pushSprite(0, 0);// dump image to display
#endif

#ifdef  USE_SD
  if ((SDcard_READY == 0) | (sensor_READY == 0)) {//get stuck here if any problem to avoid further board damage
    while (1) {

#ifndef  USE_SNEAK_MODE
      gpio_put(RED, 1);
#endif

      delay(1000);
      gpio_put(RED, 0);
      delay(1000);
    }
  }
#endif
}

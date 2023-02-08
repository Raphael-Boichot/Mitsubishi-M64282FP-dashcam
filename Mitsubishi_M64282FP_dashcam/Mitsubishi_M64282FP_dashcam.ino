//By Raphaël BOICHOT, made around 2023-01-27
//this is an autonomous recorder for the Mitsubishi M64282FP artificial retina of the Game Boy Camera
//from a code of Laurent Saint-Marcel (lstmarcel@yahoo.fr) written in 2005/07/05
//stole some code for Rafael Zenaro NeoGB printer: https://github.com/zenaro147/NeoGB-Printer
//version for Arduino here (requires a computer): https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor
//Made to be compiled on the Arduino IDE, using these libraries:
//https://github.com/earlephilhower/arduino-pico core for pi-pico
//https://github.com/Bodmer/TFT_eSPI library for TFT display
//Beware, I'm not a C developper at all so it won't be a pretty code !

//CamData[128 * 128] contains the raw signal from sensor in 8 bits
//HDRData[128 * 128] contains an average of raw signal data from sensor for dithering in 32 bits
//lookup_serial[CamData[...]] is the sensor data with autocontrast in 8 bits
//lookup_TFT_RGB565[lookup_serial[CamData[...]]] is the sensor data with autocontrast in 16 bits RGB565
//BayerData[128 * 128] contains the sensor data with dithering AND autocontrast in 2 bits
//lookup_TFT_RGB565[BayerData[...]] contains the sensor data with dithering AND autocontrast in 16 bits RGB565
//and so on...

#include "hardware/adc.h" //the GPIO commands are here
#include "big_data.h"
#include "config.h"
#include "splash.h"
#include <SPI.h>  //for SD
#include <SD.h>  //for SD

// tft directly addresses the display, im is a memory buffer for sprites
// Here I create and update a giant 128*16 sprite in memory that I push to the screen when necessary, which is ultra fast
#ifdef  USE_TFT
#include <TFT_eSPI.h> // Include the graphics library (this includes the sprite functions)
TFT_eSPI    tft = TFT_eSPI();         // Create object "tft"
TFT_eSprite img = TFT_eSprite(&tft);  // Create Sprite object "img" with pointer to "tft" object
// the pointer is used by pushSprite() to push it onto the TFT
#endif

//the ADC resolution is 0.8 mV (3.3/2^12, 12 bits) cut to 12.9 mV (8 bits), registers are close of those from the Game Boy Camera in mid light
//With these registers, the output voltage is between 0.58 and 3.04 volts (on 3.3 volts).
const char signature[20] = {};
unsigned char camReg[8] = {0b10011111, 0b11101000, 0b00000001, 0b00000000, 0b00000001, 0b000000000, 0b00000001, 0b00000011}; //registers
const unsigned char v_min = 45; //minimal voltage returned by the sensor in 8 bits DEC (0.58 volts)
const unsigned char v_max = 236;//maximal voltage returned by the sensor in 8 bits DEC (3.04 volts)
unsigned char lookup_serial[256];//autocontrast table generated in setup() from v_min and v_max
unsigned char CamData[128 * 128];// sensor data in 8 bits per pixel
unsigned char BmpData[128 * 128];// sensor data with autocontrast ready to be merged with BMP header
unsigned int HDRData[128 * 128];// cumulative data for HDR imaging -1EV, +1EV + 2xOEV, 4 images in total

#ifdef USE_DITHERING
unsigned char Bayer_matW_LG[4 * 4];//Bayer matrix to apply dithering for each image pixel white to light gray
unsigned char Bayer_matLG_DG[4 * 4];//Bayer matrix to apply dithering for each image pixel light gray to dark gray
unsigned char Bayer_matDG_B[4 * 4];//Bayer matrix to apply dithering for each image pixel dark gray to dark
unsigned char BayerData[128 * 128];// dithered image data
#endif

const double exposure_list[8] = {0.5, 0.69, 0.79, 1, 1, 1.26, 1.44, 2}; //list of exposures -1EV to +1EV by third roots of 2 steps
//const double exposure_list[8]={1, 1, 1, 1, 1, 1, 1, 1};//for fancy multi-exposure images or signal to noise ratio increasing
char num_HDR_images = sizeof(exposure_list) / sizeof( double );
const unsigned int cycles = 12; //time delay in processor cycles, to fit with the 1MHz advised clock cycle for the sensor (set with a datalogger, do not touch !)
unsigned int clock_divider = 1; //time delay in processor cycles to cheat the exposure of the sensor
const unsigned int debouncing_delay = 500; //debouncing delay for pushbuttons
unsigned long currentTime = 0;
unsigned long previousTime = 0;
unsigned long deadtime = 2000; //to introduce a deadtime for timelapses in ms. Default is 2000 ms to avoid SD card death by chocking, is read from config.txt
unsigned long Next_ID, Next_dir;//for directories and filenames
unsigned long file_number;
unsigned int current_exposure, new_exposure;
bool recording = 0;//0 = idle mode, 1 = recording mode
bool HDR_mode = 0; //0 = regular capture, 1 = HDR mode
bool BORDER_mode = 1; //1 = border enhancement ON, 0 = border enhancement OFF
bool sensor_READY = 0;
bool SDcard_READY = 0;
char storage_file_name[20], storage_file_dir[20], storage_deadtime[20], exposure_string[20], multiplier_string[20], error_string[20];

void setup()
{
  //digital stuff
  gpio_init(READ);      gpio_set_dir(READ, GPIO_IN);
  gpio_init(PUSH);      gpio_set_dir(PUSH, GPIO_IN);
  gpio_init(HDR);       gpio_set_dir(HDR, GPIO_IN);
  gpio_init(LED);       gpio_set_dir(LED, GPIO_OUT);
  gpio_init(RED);       gpio_set_dir(RED, GPIO_OUT);
  gpio_init(CLOCK);     gpio_set_dir(CLOCK, GPIO_OUT);
  gpio_init(RESET);     gpio_set_dir(RESET, GPIO_OUT);
  gpio_init(LOAD);      gpio_set_dir(LOAD, GPIO_OUT);
  gpio_init(SIN);       gpio_set_dir(SIN, GPIO_OUT);
  gpio_init(START);     gpio_set_dir(START, GPIO_OUT);

  //analog stuff
  adc_gpio_init(VOUT);  adc_select_input(0);//there are several ADC channels to choose from
  adc_init();//mandatory, without it stuck the camera

#ifdef USE_SERIAL // serial is optional, only needed for debugging or interfacing with third party soft via USB cable
  Serial.begin(2000000);
#endif

  init_sequence();//Boot screen get stuck here with red flashing LED if any problem with SD or sensor to avoid further board damage
  //now if code arrives at this point, this means that sensor and SD card are connected correctly

  deadtime = get_dead_time("/Delay.txt", deadtime);//get the dead time for timelapse from config.txt
  sprintf(storage_deadtime, "Delay: %d ms", deadtime); //concatenate string for display
  ID_file_creator("/Dashcam_storage.bin");//create a file on SD card that stores a unique file ID from 1 to 2^32 - 1 (in fact 1 to 99999)
  pre_allocate_lookup_tables(lookup_serial, v_min, v_max); //pre allocate tables for TFT and serial output auto contrast

#ifdef USE_DITHERING
  pre_allocate_Bayer_tables();// just reordering the Game Boy Camera dithering registers into 3 square matrices
#endif

#ifndef USE_FIXED_EXPOSURE
  // presets the exposure time before displaying to avoid unpleasing result, maybe be slow in the dark
  for (int i = 1; i < 10; i++) {
    take_a_picture();
    new_exposure = auto_exposure(camReg, CamData, v_min, v_max);// self explanatory
    push_exposure(camReg, new_exposure, 1); //update exposure registers C2-C3
  }
#endif
}

void loop()
{
  currentTime = millis();
  //nigth mode strategy
#ifdef NIGHT_MODE
  if (current_exposure == 0xFFFF) clock_divider = clock_divider * 2; //I reach maximum exposure = let's divide the clock frequency
  if (current_exposure < 0x1000) clock_divider = 1 ;//Normal situation is to be always 1, so that clock is about 1MHz
#endif

  take_a_picture(); //data in memory for the moment, one frame
  new_exposure = auto_exposure(camReg, CamData, v_min, v_max);// self explanatory

#ifdef  USE_FIXED_EXPOSURE
  new_exposure = FIXED_EXPOSURE;
  clock_divider = FIXED_CLOCK_DIVIDER;
#endif

  push_exposure(camReg, new_exposure, 1); //update exposure registers C2-C3

#ifdef  USE_DITHERING
  Dither_image(CamData, BayerData);
#endif


#ifdef  USE_TFT
  current_exposure = get_exposure(camReg);//get the current exposure register for TFT display
  if (current_exposure > 0x0FFF) sprintf(exposure_string, "Exposure: %X", current_exposure); //concatenate string for display
  if (current_exposure <= 0x0FFF) sprintf(exposure_string, "Exposure: 0%X", current_exposure); //concatenate string for display;
  if (current_exposure <= 0x00FF) sprintf(exposure_string, "Exposure: 00%X", current_exposure); //concatenate string for display;
  if (current_exposure <= 0x000F) sprintf(exposure_string, "Exposure: 000%X", current_exposure); //concatenate string for display;
  sprintf(multiplier_string, "Clock/%X", clock_divider); //concatenate string for display;
#endif

#ifdef USE_SERIAL
  dump_data_to_serial(CamData);//dump raw data to serial in ASCII for debugging - you can use the Matlab code ArduiCam_Matlab.m into the repo to probe the serial and plot images
#endif

#ifdef  USE_TFT
  img.fillSprite(TFT_BLACK);// prepare the image in ram
  for (int16_t x = 1; x < 128 ; x++) {
    for (int16_t y = 0; y < 120; y++) {

#ifndef  USE_DITHERING
      img.drawPixel(x, y + 16, lookup_TFT_RGB565[lookup_serial[CamData[x + y * 128]]]);//lookup_serial is autocontrast
#endif
#ifdef  USE_DITHERING
      img.drawPixel(x, y + 16, lookup_TFT_RGB565[BayerData[x + y * 128]]);//BayerData includes auto-contrast
#endif

    }
  }
#endif

  if (recording == 0) {//just put informations to the ram too

#ifdef  USE_TFT
    img.setTextColor(TFT_GREEN);
    img.setCursor(0, 8);
    img.println(F("Display Mode"));
    display_other_informations();
#endif

  }
  if (recording == 1) {// prepare data for recording mode
    if ((currentTime - previousTime) > deadtime) {// Wait for deadtime set in config.txt
      Next_ID++;// update the file number
      previousTime = currentTime;
      sprintf(storage_file_name, "/%06d/%09d.bmp", Next_dir, Next_ID);//update filename

      if (HDR_mode == 1) {//default is 8 pictures, beware of modifying the code in case of change

#ifdef  USE_TFT
        img.setTextColor(TFT_BLUE);
        img.setCursor(0, 16);
        img.println(F("HDR in acquisition"));
        display_other_informations();
        img.pushSprite(0, 0);// dump image to display
#endif

        gpio_put(RED, 1);
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

#ifndef  USE_DITHERING
      for (int i = 0; i < 128 * 128; i++) {
        BmpData[i] = lookup_serial[CamData[i]];//to get data with autocontrast
      }
#endif

#ifdef  USE_DITHERING
      Dither_image(CamData, BayerData);
      for (int i = 0; i < 128 * 128; i++) {
        BmpData[i] = BayerData[i];//to get data with dithering (dithering includes auto-contrast)
      }
#endif
      gpio_put(RED, 1);
      File dataFile = SD.open(storage_file_name, FILE_WRITE);
      // if the file is writable, write to it:
      if (dataFile) {
        dataFile.write(BMP_header, 1078);//fixed header for 128*120 image
        dataFile.write(BmpData, 128 * 120); //removing last tile line
        dataFile.close();
      }
      gpio_put(RED, 0);
    }
#ifdef  USE_TFT
    img.setTextColor(TFT_RED);
    img.setCursor(0, 8);
    img.println(F("Recording Mode"));
    display_other_informations();
#endif

    if (deadtime > 10000) sleep_ms(2000); //for timelapses with long deadtimes, no need to constantly spam the sensor for autoexposure
  }

#ifdef  USE_TFT
  img.pushSprite(0, 0);// dump image to display
#endif

  if ((gpio_get(PUSH) == 1) && recording == 0) { // we want to record: get file/directory#
    Next_ID = get_next_ID("/Dashcam_storage.bin");//get the file number on SD card
    Next_dir = get_next_dir("/Dashcam_storage.bin");//get the folder number on SD card
    sprintf(storage_file_dir, "/%06d/", Next_dir);//update next directory
    SD.mkdir(storage_file_dir);//create next directory
    gpio_put(RED, 1);
    recording = 1;
    previousTime = currentTime;//To avoid taking a picture while pressing the mode button
    delay(debouncing_delay);//for debouncing
    gpio_put(RED, 0);
  }

  if ((gpio_get(PUSH) == 1) && recording == 1) { // we want to stop recording
    Next_dir++;// update next directory
    store_next_ID("/Dashcam_storage.bin", Next_ID, Next_dir);//store last known file/directory# to SD card
    gpio_put(RED, 0);
    recording = 0;
    delay(debouncing_delay);//get the folder number on SD card
  }

  if (recording == 0) { // Change HDR<->one frame modes
    if (gpio_get(HDR) == 1) {
      HDR_mode = !HDR_mode;
      delay(debouncing_delay);
    }
    if (gpio_get(BORDER) == 1) {// Change raw<->2D enhanced images
      BORDER_mode = !BORDER_mode;
      if (BORDER_mode == 1) camReg[1] = 0b11101000;//With 2D border enhancement
      if (BORDER_mode == 0) camReg[1] = 0b00001000;//Without 2D border enhancement (very soft image, better for nightmode)
      delay(debouncing_delay);
    }
  }
} //end of loop

//////////////////////////////////////////////Sensor stuff///////////////////////////////////////////////////////////////////////////////////////////
void take_a_picture() {
  camReset();// resets the sensor
  camSetRegisters();// Send 8 registers to the sensor
  camReadPicture(CamData); // get pixels, dump them in RawCamData
  camReset();
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
  if ((error <= 10) && (error >= 2))   new_regs = exp_regs + 1;
  if ((error >= -10) && (error <= -2))  new_regs = exp_regs - 1;
  sprintf(error_string, "Error: %d", int(error)); //concatenate string for display;
  return int(new_regs);
}

void push_exposure(unsigned char camReg[8], unsigned int current_exposure, double factor) {
  double new_regs;
  new_regs = current_exposure * factor;
  if (new_regs < 0x0010) {//minimum of the sensor, below there are verticals artifacts
    new_regs = 0x0010;
  }
  if (new_regs > 0xFFFF) {//maximum of the sensor, about 1 second
    new_regs = 0xFFFF;
  }
  camReg[2] = int(new_regs / 256);
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
    if (regaddr & bitmask)
    {
      gpio_put(SIN, 1);// Set the SIN bit
    }
    else
    {
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
    if (regval & bitmask)
    {
      gpio_put(SIN, 1);// set the SIN bit
    }
    else
    {
      gpio_put(SIN, 0);
    }
    camDelay();
    if (bitmask == 1)
      gpio_put(LOAD, 1);// Assert load at rising edge of CLOCK
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
  gpio_put(LED, 1);
  while (1) {// Wait for READ to go high
    gpio_put(CLOCK, 1);
    camSpecialDelay();
    if (gpio_get(READ) == 1) // READ goes high with rising CLOCK
      break;
    camDelay();
    gpio_put(CLOCK, 0);
    camSpecialDelay();
  }
  camDelay();
  gpio_put(LED, 0);
  for (y = 0; y < 128; y++) {
    for (x = 0; x < 128; x++) {
      gpio_put(CLOCK, 0);
      camDelay();
      pixel = adc_read();// The ADC is 12 bits, this sacrifies the 4 least significant bits to simplify transmission
      CamData[subcounter] = pixel >> 4;
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

bool camTestSensor() // dummy cycle faking to take a picture, if it's not able to go through the whole cycle, there is an issue
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
  gpio_put(LED, 1);
  currentTime = millis();
  while (1) {// Wait for READ to go high
    gpio_put(CLOCK, 1);
    camDelay();
    if (gpio_get(READ) == 1) break;// READ goes high with rising CLOCK, everything is OK
    if ((millis() - currentTime) > 1000) {
      sensor_OK = 0;
      break;//the sensor does not respond after 1 second = not connected
    }
    camDelay();
    gpio_put(CLOCK, 0);
    camDelay();
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
void pre_allocate_lookup_tables(unsigned char lookup_serial[256], unsigned char v_min, unsigned char v_max) {
  double gamma_pixel;
  for (int i = 0; i < 256; i++) {//first the autocontrat table lookup_serial
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
#ifdef USE_DITHERING
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
#endif
}

void Dither_image(unsigned char CamData[128 * 128], unsigned char BayerData[128 * 128])
{
#ifdef USE_DITHERING
  //very minimal dithering algorithm
  char pixel, pixel_out;
  char W = 255; //white as it will apear on the display and in bmop file
  char LG = 170; //light gray white as it will apear on the display and in bmop file
  char DG = 110; //dark gray white as it will apear on the display and in bmop file
  char B = 0; //black white as it will apear on the display and in bmop file
  int counter = 0;
  pixel_out = 0;
  for (int y = 0; y < 128; y++) {
    for (int x = 0; x < 128; x++) {
      pixel = lookup_serial[CamData[counter]];//auto_contrasted values, may range between 0 and 255
      if (pixel < DG) {
        if (pixel > Bayer_matDG_B[(x & 3) + 4 * (y & 3)]) pixel_out = DG;
        else pixel_out = B;
      }
      if ((pixel >= LG) & (pixel < DG)) {
        if (pixel > Bayer_matLG_DG[(x & 3) + 4 * (y & 3)]) pixel_out = LG;
        else pixel_out = DG;
      }
      if (pixel >= DG ) {
        if (pixel > Bayer_matW_LG[(x & 3) + 4 * (y & 3)]) pixel_out = W;
        else pixel_out = LG;
      }
      counter = counter + 1;
      BayerData[counter] = pixel_out;
    }
  }
#endif
}

void dump_data_to_serial(unsigned char CamData[128 * 128]) {
  char pixel;
  for (int i = 0; i < 128 * 128; i++) {
    pixel = lookup_serial[CamData[i]];//to get data with autocontrast
    //pixel = CamData[i]; //to get data without autocontrast
    if (pixel <= 0x0F) Serial.print('0');
    Serial.print(pixel, HEX);
    Serial.print(" ");
  }
  Serial.println("");
}

//////////////////////////////////////////////SD stuff///////////////////////////////////////////////////////////////////////////////////////////
void ID_file_creator(const char * path) {
  uint8_t buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  if (!SD.exists(path)) {
    File file = SD.open(path, FILE_WRITE);
    //start from a fresh install on SD
    file.write(buf, 8);
    file.close();
  }
}

unsigned long get_next_ID(const char * path) {
  uint8_t buf[4];
  File file = SD.open(path);
  file.read(buf, 4);
  Next_ID = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]));
  return Next_ID;
  file.close();
}

unsigned long get_next_dir(const char * path) {
  uint8_t buf[4];
  File file = SD.open(path);
  file.read(buf, 4);//dumps the 4 first bytes
  file.read(buf, 4);
  Next_dir = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]));
  return Next_dir;
  file.close();
}

unsigned long get_dead_time(const char * path, unsigned long deadtime) {
  unsigned long delay_timelapse = deadtime;

  if (SD.exists(path)) {
    deadtime = 0;
    unsigned char table[20];
    File file = SD.open(path);
    unsigned char i = 0;
    while (file.available()) {
      table[i] = ASCII_to_num[file.read()];
      i = i + 1;
    }
    for (int pos = 0; pos < i; pos++) {
      deadtime = deadtime + table[pos] * pow(10, i - pos - 1);//I know, it's ridiculous
    }
    delay_timelapse = deadtime;
    file.close();
  }
  return delay_timelapse;
}

void store_next_ID(const char * path, unsigned long Next_ID, unsigned long Next_dir) {
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
}

//////////////////////////////////////////////Display stuff///////////////////////////////////////////////////////////////////////////////////////////
void display_other_informations() {
#ifdef  USE_TFT
  img.setTextColor(TFT_CYAN);
  img.setCursor(0, 0);
  img.println(F(exposure_string));
  img.setTextColor(TFT_BLUE);
  img.setCursor(64, 128);
  img.println(F(error_string));
  img.setCursor(0, 128);
  img.println(F(multiplier_string));
  img.setTextColor(TFT_WHITE);
  img.setCursor(0, 136);
  img.println(F(storage_file_name));
  img.setTextColor(TFT_BLUE);
  img.setCursor(0, 144);
  img.println(F(storage_deadtime));
  img.setCursor(0, 152);
  if (HDR_mode == 1) {
    img.setTextColor(TFT_RED);
    img.println(F("HDR mode ON"));
  }
  if (HDR_mode == 0) {
    img.setTextColor(TFT_GREEN);
    img.println(F("HDR mode OFF"));
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
  img.println(F("SD card:"));
  img.pushSprite(0, 0);// dump image to display
#endif

  //see if the card is present and can be initialized
  if (SD.begin(CHIPSELECT)) {
    SDcard_READY = 1;
  }
  else {
    SDcard_READY = 0;
  }

#ifdef  USE_TFT
  if (SDcard_READY == 1) {
    img.setTextColor(TFT_GREEN);
    img.setCursor(50, 0);
    img.println(F("READY"));
  }
  else {
    img.setTextColor(TFT_RED);
    img.setCursor(50, 0);
    img.println(F("FAIL"));
  }
  img.setTextColor(TFT_WHITE);
  img.setCursor(0, 8);
  img.println(F("Sensor:"));
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
    img.println(F("READY"));
  }
  else {
    img.setTextColor(TFT_RED);
    img.setCursor(50, 8);
    img.println(F("FAIL"));
  }

  if ((SDcard_READY == 0) | (sensor_READY == 0)) {
    img.setTextColor(TFT_RED);
    img.setCursor(0, 16);
    img.println(F("CHECK CONNECTIONS"));
  }
  else {
    img.setTextColor(TFT_GREEN);
    img.setCursor(0, 16);
    img.println(F("NOW BOOTING..."));
  }
  img.pushSprite(0, 0);// dump image to display
#endif
  if ((SDcard_READY == 0) | (sensor_READY == 0)) {//get stuck here if any problem to avoid further board damage
    while (1) {
      gpio_put(RED, 1);
      delay(1000);
      gpio_put(RED, 0);
      delay(1000);
    }
  }
}

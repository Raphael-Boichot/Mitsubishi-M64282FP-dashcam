//By Raphaël BOICHOT, made around 2023-04-27
//this version is to use with the Matlab code to test the effect of registers
//it outputs data over the serial and diplays raw data on screen

#include "hardware/adc.h" //the GPIO commands are here

#define NOP __asm__ __volatile__ ("nop\n\t") //// minimal possible delay

#include <TFT_eSPI.h>                 // Include the graphics library (this includes the sprite functions)
TFT_eSPI    tft = TFT_eSPI();         // Create object "tft"
TFT_eSprite img = TFT_eSprite(&tft);  // Create Sprite object "img" with pointer to "tft" object
// the pointer is used by pushSprite() to push it onto the TFT
#define BITS_PER_PIXEL 16             // How many bits per pixel in Sprite, here RGB565 format

// the order of pins has no importance except that VOUT must be on some ADC
const uint VOUT =  26; //to pi pico pin GPIO26/A0 Analog signal from sensor, read shortly after clock is set low, native 12 bits, converted to 8 bits

//the following pins must be shifted to 3.3V pico side<->5V sensor side
const uint READ =  7;   //to pi pico pin GPIO7 Read image signal, goes high on rising clock
const uint CLOCK = 8;   //to pi pico pin GPIO8 Clock input, pulled down internally, no specification given for frequency
const uint RESET = 9;   //to pi pico pin GPIO9 system reset, pulled down internally, active low, sent on rising edge of clock
const uint LOAD =  10;  //to pi pico pin GPIO10 parameter set enable, pulled down internally, Raise high as you clear the last bit of each register you send
const uint SIN =   11;  //to pi pico pin GPIO11 Image sensing start, pulled down internally, has to be high before rising CLOCK
const uint START = 12;  //to pi pico pin GPIO12 Image sensing start, pulled down internally, has to be high before rising CLOCK
const uint LED =   15; //to pi pico pin GPIO15 indicate exposure delay for the sensor <-> GND
// it is advised to attach pi pico pin RUN pin to any GND via a pushbutton for resetting the pico

/*
  reg0=Z1 Z0 O5 O4 O3 O2 O1 O0 zero point calibration and output reference voltage
  reg1=N VH1 VH0 G4 G3 G2 G1 G0
  reg2=C17 C16 C15 C14 C13 C12 C11 C10 / exposure time by 4096 ms steps (max 1.0486 s)
  reg3=C07 C06 C05 C04 C03 C02 C01 C00 / exposure time by 16 µs steps (max 4096 ms)
  reg4=P7 P6 P5 P4 P3 P2 P1 P0 filtering kernels
  reg5=M7 M6 M5 M4 M3 M2 M1 M0 filtering kernels
  reg6=X7 X6 X5 X4 X3 X2 X1 X0 filtering kernels
  reg7=E3 E2 E1 E0 I V2 V1 V0 set Vref
*/
//V (Vref, raw tuning) and O (output ref voltage, fine tuning) are voltage offset registers. They are additive, but only if V is not equal to zero.
// this plus gain allow covering the whole voltage scale of the ADC (about 3.3 volts here, maybe more)

//unsigned char camReg[8] = {0b10000000, 0b11100000, 0b11111111, 0b11111111, 0b00000001, 0b000000000, 0b00000001, 0b00000000}; //registers
unsigned char camReg[8] = {0b10011111, 0b11101000, 0b11111111, 0b11111111, 0b00000001, 0b000000000, 0b00000001, 0b00000011}; //registers
//registers with no border enhancement (gives very soft image)
//unsigned char camReg[8] = {0b10000000, 0b00000000, 0b00000000, 0b00000100, 0b00000001, 0b000000000, 0b00000001, 0b00000000}; //registers
//unsigned char camReg[8] = {0b10011111, 0b00001000, 0b11111111, 0b11111111, 0b00000001, 0b000000000, 0b00000001, 0b00000011}; //registers

unsigned char lookup_serial[256];//loockup table for serial output
const unsigned char LUT_serial[16] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46}; //HEX characters in ASCII table
//lookup table to read config.txt
unsigned char reg, char1, char2;
unsigned char CamData[128 * 128];// sensor data in 8 bits per pixel
const int cycles = 12; //time delay in processor cycles, to fit with the 1MHz advised clock cycle for the sensor (set with a datalogger, do not touch !)

void setup()
{
  //digital stuff
  gpio_init(READ);      gpio_set_dir(READ, GPIO_IN);
  gpio_init(LED);       gpio_set_dir(LED, GPIO_OUT);
  gpio_init(CLOCK);     gpio_set_dir(CLOCK, GPIO_OUT);
  gpio_init(RESET);     gpio_set_dir(RESET, GPIO_OUT);
  gpio_init(LOAD);      gpio_set_dir(LOAD, GPIO_OUT);
  gpio_init(SIN);       gpio_set_dir(SIN, GPIO_OUT);
  gpio_init(START);     gpio_set_dir(START, GPIO_OUT);

  //analog stuff
  adc_gpio_init(VOUT);  adc_select_input(0);
  adc_init();

  // tft directly addresses the display, im is a memory buffer for sprites
  // Here I create and update a giant 128*16 sprite in memory that I push to the screen when necessary, which is ultra fast
  tft.init();
  tft.setRotation(2);
  img.fillScreen(TFT_BLACK);
  img.setColorDepth(BITS_PER_PIXEL);         // Set colour depth first
  img.createSprite(128, 160);                // then create the sprite
  img.pushSprite(0, 0);                      // direct memory transfer to the display

  Serial.begin(2000000);
}

void loop()
{
 take_a_picture(); //data in memory for the moment, one frame
 dump_data_to_serial(CamData);//dump data to serial for debugging - you can use the Matlab code ArduiCam_Matlab.m into the repo to probe the serial and plot images
  img.fillSprite(TFT_BLACK);// prepare the image in ram
  for (int16_t x = 1; x < 128 ; x++) {
    for (int16_t y = 0; y < 128; y++) {
      img.drawPixel(x, y + 16, lookup_TFT_RGB565(CamData[x + y * 128]));
    }
  }
  img.pushSprite(0, 0);  
delay(500);
} //end of loop

//////////////////////////////////////////////Sensor stuff///////////////////////////////////////////////////////////////////////////////////////////
void take_a_picture() {
  camReset();// resets the sensor
  camSetRegisters();// Send 8 registers to the sensor
  camReadPicture(CamData); // get pixels, dump them in RawCamData
  camReset();
}

void camDelay()// Allow a lag in processor cycles to maintain signals long enough
{
  for (int i = 0; i < cycles; i++) NOP;
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
  for (reg = 0; reg < 8; ++reg) {
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
    camDelay();
    if (gpio_get(READ) == 1) // READ goes high with rising CLOCK
      break;
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

unsigned int lookup_TFT_RGB565(char color){
  char red=color;
  char green=color;
  char blue=color;
  unsigned int Rgb565 = (((red & 0b11111000) << 8) + ((green & 0b11111100) << 3) + (blue >> 3));
  return Rgb565;
}

void dump_data_to_serial(unsigned char CamData[128 * 128]) {
  char pixel;
  for (int i = 0; i < 128 * 128; i++) {
    //pixel = lookup_serial[CamData[i]];//to get data with autocontrast
    pixel = CamData[i]; //to get data without autocontrast
    if(pixel<=0x0F) Serial.print('0');
    Serial.print(pixel,HEX);
    Serial.print(" ");
  }
  Serial.println("");
}

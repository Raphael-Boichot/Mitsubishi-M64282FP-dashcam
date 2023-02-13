#define USE_TFT //to allow using the TFT screen
#define USE_SD //to allow recording on SD
//#define USE_SERIAL //mode for outputing image in ascii to the serial console
//#define USE_EXTREME_OVERCLOCK //Use at your own risks !!! Twice faster but out of tolerance for everything and unstable

//See for details https://github.com/HerrZatacke/dither-pattern-gen/ and https://herrzatacke.github.io/dither-pattern-gen/
unsigned char Dithering_patterns [] = { 0x2A, 0x5E, 0x9B, 0x51, 0x8B, 0xCA, 0x33, 0x69, 0xA6, 0x5A, 0x97, 0xD6, 0x44, 0x7C, 0xBA, 0x37, 0x6D, 0xAA, 0x4D, 0x87, 0xC6, 0x40, 0x78, 0xB6, 0x30, 0x65, 0xA2, 0x57, 0x93, 0xD2, 0x2D, 0x61, 0x9E, 0x54, 0x8F, 0xCE, 0x4A, 0x84, 0xC2, 0x3D, 0x74, 0xB2, 0x47, 0x80, 0xBE, 0x3A, 0x71, 0xAE };
double exposure_list[8] = {0.5, 0.69, 0.79, 1, 1, 1.26, 1.44, 2}; //list of exposures -1EV to +1EV by third roots of 2 steps for HDR mode
unsigned char Dithering_palette[4] = {0x00, 0x55, 0xAA, 0xFF};//colors as they will appear in the bmp file and display after dithering
//const double exposure_list[8]={1, 1, 1, 1, 1, 1, 1, 1};//for fancy multi-exposure images or signal to noise ratio increasing

#define NOP __asm__ __volatile__ ("nop\n\t") //// minimal possible delay
#define BITS_PER_PIXEL 16             // How many bits per pixel in Sprite, here RGB565 format

// the order of pins has no importance except that VOUT must be on some ADC
#define VOUT 26 //to pi pico pin GPIO26/A0 Analog signal from sensor, read shortly after clock is set low, native 12 bits, converted to 8 bits

//the following pins must be shifted to 3.3V pico side<->5V sensor side
#define READ 7    //to pi pico pin GPIO7 Read image signal, goes high on rising clock
#define CLOCK 8   //to pi pico pin GPIO8 Clock input, pulled down internally, no specification given for frequency
#define RESET 9   //to pi pico pin GPIO9 system reset, pulled down internally, active low, sent on rising edge of clock
#define LOAD 10   //to pi pico pin GPIO10 parameter set enable, pulled down internally, Raise high as you clear the last bit of each register you send
#define SIN 11    //to pi pico pin GPIO11 Image sensing start, pulled down internally, has to be high before rising CLOCK
#define START 12  //to pi pico pin GPIO12 Image sensing start, pulled down internally, has to be high before rising CLOCK

//the following are intended to allow interfering with the device
#define LED 15    //to pi pico pin GPIO15 indicate exposure delay for the sensor <-> GND
#define RED 14    //to pi pico pin GPIO14 indicate recording to SD card of issue with SD card <-> GND
#define PUSH 13   //to pi pico pin GPIO13 action button <-> 3.3V
#define HDR 20    //to pi pico pin GPIO20 <-> 3.3V
#define BORDER 22 //to pi pico pin GPIO22 <-> 3.3V
#define DITHER 21 //to pi pico pin GPIO22 <-> 3.3V
// it is advised to attach pi pico pin RUN pin to any GND via a pushbutton for resetting the pico

//Beware, SD card MUST be attached to these pins as the pico seems not very tolerant with SD card pinout, they CANNOT be changed
//   SD_MISO - to pi pico pin GPIO16
//   SD_MOSI - to pi pico pin GPIO19
//   SD_CS   - to pi pico pin GPIO17
//   SD_SCK  - to pi pico pin GPIO18
#define CHIPSELECT 17//for SD card, but I recommend not changing it either

//TFT screens pins are more flexible, I used a 1.8 TFT SPI 128*160 V1.1 model (ST7735 driver)
// pins are configured into the Bodmer TFT e_SPI library, DO NOT CHANGE HERE, see read.me for details
// Display LED       to pi pico pin 3V3
// Display SCK       to pi pico pin GPIO2
// Display SDA       to pi pico pin GPIO3
// Display CS        to pi pico pin GPIO4 (can use another pin if desired)
// Display AO        to pi pico pin GPIO5 (can use another pin if desired)
// Display RESET     to pi pico pin GPIO6 (can use another pin if desired)
// Display GND       to pi pico pin GND (0V)
// Display VCC       to pi pico pin VBUS Or +5V

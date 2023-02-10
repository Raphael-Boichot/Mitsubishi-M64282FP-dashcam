
#define USE_TFT //to allow using the TFT screen
#define USE_SD //to allow recording on SD
//#define USE_SERIAL //mode for outputing image in ascii to the serial console
//#define USE_FIXED_EXPOSURE //mode for astrophotography
//#define USE_EXTREME_OVERCLOCK //Use at your own risks !!! Twice faster but out of tolerance for everything and unstable

//See for details https://github.com/HerrZatacke/dither-pattern-gen/ and https://herrzatacke.github.io/dither-pattern-gen/
unsigned char Dithering_patterns [] = {0x20, 0x5F, 0xA3, 0x4F, 0x92, 0xCF, 0x2B, 0x6B, 0xAE, 0x5B, 0x9E, 0xDA, 0x3F, 0x81, 0xC0, 0x2F, 0x70, 0xB1, 0x4B, 0x8D, 0xCB, 0x3B, 0x7C, 0xBC, 0x27, 0x67, 0xAA, 0x57, 0x9A, 0xD6, 0x23, 0x63, 0xA6, 0x53, 0x96, 0xD2, 0x47, 0x89, 0xC7, 0x37, 0x78, 0xB9, 0x43, 0x85, 0xC4, 0x33, 0x74, 0xB5 };

double exposure_list[8] = {0.5, 0.69, 0.79, 1, 1, 1.26, 1.44, 2}; //list of exposures -1EV to +1EV by third roots of 2 steps for HDR mode
//const double exposure_list[8]={1, 1, 1, 1, 1, 1, 1, 1};//for fancy multi-exposure images or signal to noise ratio increasing

#ifdef  USE_FIXED_EXPOSURE //here the result is a fixed exposure perfect for full moon photography
#define FIXED_EXPOSURE 2048
#define FIXED_CLOCK_DIVIDER 1
#endif

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

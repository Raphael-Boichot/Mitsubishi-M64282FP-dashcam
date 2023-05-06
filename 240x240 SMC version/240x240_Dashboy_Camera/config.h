#define USE_TFT //to allow using the TFT screen
#define USE_SD //to allow recording on SD
//#define ST7735 //for use with the TFT 128x160
#define ST7789 //for use with the TFT 240x240
//#define USE_OVERCLOCKING //self explanatory
//#define USE_SERIAL //mode for outputing image in ascii to the serial console
//#define USE_SNEAK_MODE //deactivates the LEDs
//#define DEBUG_MODE //allow using the serial

#ifdef  ST7789
#define x_ori 56
#define y_ori 40
#endif

#ifdef  ST7735
#define x_ori 0
#define y_ori 0
#endif

//See for details https://github.com/HerrZatacke/dither-pattern-gen/ and https://herrzatacke.github.io/dither-pattern-gen/
unsigned char Dithering_patterns_regular[] = { 0x2A, 0x5E, 0x9B, 0x51, 0x8B, 0xCA, 0x33, 0x69, 0xA6, 0x5A, 0x97, 0xD6, 0x44, 0x7C, 0xBA, 0x37, 0x6D, 0xAA, 0x4D, 0x87, 0xC6, 0x40, 0x78, 0xB6, 0x30, 0x65, 0xA2, 0x57, 0x93, 0xD2, 0x2D, 0x61, 0x9E, 0x54, 0x8F, 0xCE, 0x4A, 0x84, 0xC2, 0x3D, 0x74, 0xB2, 0x47, 0x80, 0xBE, 0x3A, 0x71, 0xAE };
unsigned char Dithering_patterns_high[] = { 0x89, 0x92, 0xA2, 0x8F, 0x9E, 0xC6, 0x8A, 0x95, 0xAB, 0x91, 0xA1, 0xCF, 0x8D, 0x9A, 0xBA, 0x8B, 0x96, 0xAE, 0x8F, 0x9D, 0xC3, 0x8C, 0x99, 0xB7, 0x8A, 0x94, 0xA8, 0x90, 0xA0, 0xCC, 0x89, 0x93, 0xA5, 0x90, 0x9F, 0xC9, 0x8E, 0x9C, 0xC0, 0x8C, 0x98, 0xB4, 0x8E, 0x9B, 0xBD, 0x8B, 0x97, 0xB1 };
unsigned char Dithering_patterns_low[] = { 0x8C, 0x98, 0xAC, 0x95, 0xA7, 0xDB, 0x8E, 0x9B, 0xB7, 0x97, 0xAA, 0xE7, 0x92, 0xA2, 0xCB, 0x8F, 0x9D, 0xBB, 0x94, 0xA5, 0xD7, 0x91, 0xA0, 0xC7, 0x8D, 0x9A, 0xB3, 0x96, 0xA9, 0xE3, 0x8C, 0x99, 0xAF, 0x95, 0xA8, 0xDF, 0x93, 0xA4, 0xD3, 0x90, 0x9F, 0xC3, 0x92, 0xA3, 0xCF, 0x8F, 0x9E, 0xBF };
char dithering_strategy[] = {2, 1, 1, 1, 1, 0}; //2 for regular strategy, 0 for Dithering_patterns_low, 1 Dithering_patterns_high, from high to low light
double exposure_list[8] = {0.5, 0.69, 0.79, 1, 1, 1.26, 1.44, 2}; //list of exposures -1EV to +1EV by third roots of 2 steps for HDR mode
//double exposure_list[8]={1, 1, 1, 1, 1, 1, 1, 1};//for fancy multi-exposure images or signal to noise ratio increasing
double timelapse_list[8] = { -1, 0, 1000, 2000 , 4000, 8000, 16000, -2};//-2 = motion sensor mode, -1 = regular mode, value >=0 = time in ms
unsigned char Dithering_palette[4] = {0x00, 0x55, 0xAA, 0xFF};//colors as they will appear in the bmp file and display after dithering
double motion_detection_threshold = 0.025;
double difference_threshold; //trigger threshold for motion sensor

//default values in case config.json is not existing/////////////////////////////////////////////////////////////////////////////////////////////
bool TIMELAPSE_mode = 0;//0 = use s a regular camera, 1 = recorder for timelapses
bool RAW_recording_mode = 0; //0 = save as BMP (slow), 1 = save as raw stream (fast)
unsigned long TIMELAPSE_deadtime = 0; //to introduce a deadtime for timelapses in ms, is read from config.json
char PRETTYBORDER_mode = 1;//0 = 128*120 image, 1 = 128*114 image + 160*144 border, like the GB Camera
bool NIGHT_mode = 0; //0 = exp registers cap to 0xFFFF, 1 = clock hack. I'm honestly not super happy of the current version but it works
bool HDR_mode = 0; //0 = regular capture, 1 = HDR mode
bool GBCAMERA_mode = 0; // 0 = single register strategy, 1 = Game Boy Camera strategy
bool DITHER_mode = 0; //0 = Dithering ON, 0 = dithering OFF
bool BORDER_mode = 1; //1 = border enhancement ON, 0 = border enhancement OFF. On by default because image is very blurry without
bool FIXED_EXPOSURE_mode = 0;// to activate fixed exposure delay mode
int FIXED_delay = 2048;//here the result is a fixed exposure perfect for full moon photography
int FIXED_divider = 1;//clock divider

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
#define LED 0    //to pi pico pin GPIO0 indicate exposure delay for the sensor <-> GND
#define RED 1    //to pi pico pin GPIO1 indicate recording to SD card of issue with SD card <-> GND
#define PUSH 13   //to pi pico pin GPIO13 action button <-> 3.3V - action button to record
#define TLC 14    //to pi pico pin GPIO14 <-> 3.3V - timelapse<->regular camera mode
#define LOCK 15   //to pi pico pin GPIO15  <-> 3.3V - Lock exposure
#define HDR 20     //to pi pico pin GPIO20 <-> 3.3V - HDR mode
#define DITHER 21 //to pi pico pin GPIO21 <-> 3.3V - dithering with Bayer matrix
#define INT 25    //to pi pico pin GPIO25 internal LED;
//#define XXX 22    //pin GPIO22 is the only left;
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
// Display TFT_MISO  to pi pico pin GPIO99  // RESERVED BUT NOT USED, just to avoid a compiling message, can be GPIO0
// Display TOUCH_CS  to pi pico pin GPIO99  // RESERVED BUT NOT USED, just to avoid a compiling message, can be GPIO1
// Display SCK       to pi pico pin GPIO2
// Display SDA       to pi pico pin GPIO3
// Display CS        to pi pico pin GPIO4 (can use another pin if desired)
// Display AO        to pi pico pin GPIO5 (can use another pin if desired)
// Display RESET     to pi pico pin GPIO6 (can use another pin if desired)
// Display GND       to pi pico pin GND (0V)
// Display VCC       to pi pico pin VBUS Or +5V

// some real settings used by the Mitsubishi sensor on Game Boy Camera, except exposure
// reg0 = 0b10011111; % Z1 Z0 O5 O4 O3 O2 O1 O0 zero point calibration / output reference voltage fine -> O and V add themselves, if !V==0
// reg1 = 0b11100100; % N VH1 VH0 G4 G3 G2 G1 G0 set edge / type of edge
// reg2 = 0b00000001; % C17 C16 C15 C14 C13 C12 C11 C10 / exposure time by 4096 ms steps (max 1.0486 s)
// reg3 = 0b00000000; % C07 C06 C05 C04 C03 C02 C01 C00 / exposure time by 16 µs steps (max 4096 ms)
// reg4 = 0b00000001; % P7 P6 P5 P4 P3 P2 P1 P0 filtering kernels, always 0x01 on a GB camera, but can be different here
// reg5 = 0b00000000; % M7 M6 M5 M4 M3 M2 M1 M0 filtering kernels, always 0x00 on a GB camera, but can be different here
// reg6 = 0b00000001; % X7 X6 X5 X4 X3 X2 X1 X0 filtering kernels, always 0x01 on a GB camera, but can be different here
// reg7 = 0b00000011; % E3 E2 E1 E0 I V2 V1 V0 Edge enhancement ratio / invert / Output node bias voltage raw -> O and V add themselves, if !V==0

// P, M and X registers allows pure edge extraction for example, see datasheet, all other registers must be modified accordingly

//Game Boy Camera strategy: uses half the voltage scale
///////////////////////////{0bZZOOOOOO, 0bNVVGGGGG, 0bCCCCCCCC, 0bCCCCCCCC, 0bPPPPPPPP, 0bMMMMMMMM, 0bXXXXXXXX, 0bEEEEIVVV};
unsigned char camReg1[8] = {0b10101001, 0b00100000, 0b00000000, 0b00000000, 0b00000001, 0b00000000, 0b00000001, 0b00000011};//low exposure time - high light
//transition@ C=0x0030
unsigned char camReg2[8] = {0b10101001, 0b11100000, 0b00000000, 0b00000000, 0b00000001, 0b00000000, 0b00000001, 0b00000011};
//transition@ C=0x0D80
unsigned char camReg3[8] = {0b10101011, 0b11100100, 0b00000000, 0b00000000, 0b00000001, 0b00000000, 0b00000001, 0b00000011};
//transition@ C=0x3500
unsigned char camReg4[8] = {0b10101111, 0b11101000, 0b00000000, 0b00000000, 0b00000001, 0b00000000, 0b00000001, 0b00000011};
//transition@ C=0x8500
unsigned char camReg5[8] = {0b10100111, 0b00001010, 0b00000000, 0b00000000, 0b00000001, 0b00000000, 0b00000001, 0b00000011};//high exposure time - low light
//It is assumed that usefull range is between 1.5 and 3.0 volts, so between 116 and 232
unsigned char GB_v_min = 125; //minimal voltage returned by the sensor in 8 bits DEC (1.5 volts is 112 but 125 gives better black)
unsigned char GB_v_max = 210;//maximal voltage returned by the sensor in 8 bits DEC (3.05 volts is 236 but 210 gives better white)
/////////////////////////

//DashBoy Camera regular strategy: uses the whole voltage scale
//the ADC resolution is 0.8 mV (3.3/2^12, 12 bits) cut to 12.9 mV (8 bits), registers are close of those from the Game Boy Camera in mid light
//With these registers, the output voltage is between 0.58 and 3.04 volts (on 3.3 volts), this is the best I can do.
//////////////////////////{0bZZOOOOOO, 0bNVVGGGGG, 0bCCCCCCCC, 0bCCCCCCCC, 0bPPPPPPPP, 0bMMMMMMMM, 0bXXXXXXXX, 0bEEEEIVVV};
unsigned char camReg[8] = {0b10011111, 0b11101000, 0b00000001, 0b00000000, 0b00000001, 0b000000000, 0b00000001, 0b00000011}; //registers

unsigned char regular_v_min = 55; //minimal voltage returned by the sensor in 8 bits DEC (0.58 volts is 45 but 55 gives better black)
unsigned char regular_v_max = 236;//maximal voltage returned by the sensor in 8 bits DEC (3.05 volts)
////////////////////////

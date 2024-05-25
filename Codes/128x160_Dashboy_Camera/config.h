#define USE_TFT  //to allow using the TFT screen - deactivate for debug
#define USE_SD   //to allow recording on SD - deactivate for debug
#define ST7735   //for use with the TFT 128x160 (full frame image), default setup
//#define ST7789 //for use with the minuscule TFT 240x240 (128x160 image centered, no crop) - Beware, you also have to modify the TFT setup accordingly
//#define TADDREGISTER  //additional support for the Mitsubishi M64283FP CMOS sensor, untested !
//#define USE_OVERCLOCKING  //self explanatory, use with the Arduino IDE overclocking option to 250 MHz, beware, it changes the sensor clock parameters
//#define USE_SERIAL //mode for outputing image in ascii to the serial console
//#define USE_SNEAK_MODE  //deactivates the LEDs, why not
#define DEBUG_MODE     //allow additionnal outputs on display
#define DEBAGAME_MODE  //more variables: masked pixel, O reg and V reg voltages

#ifdef ST7735  //natural screen to use, 128x160 pixels, all acreen used, pixel perfect rendering
#define x_ori 0
#define y_ori 0
#endif

#ifdef ST7789  //possible other screen, 256x256 pixels, very tiny, pixel perfect but image centered on a post stamp...
#define x_ori 56
#define y_ori 40
#endif

///////////////default values in case config.json is not existing////////////////////////////////////////////////////////////////////
bool RAW_recording_mode = 0;                                              //0 = save as BMP (slow), 1 = save as raw stream (fast)
double timelapse_list[8] = { -1, 0, 1000, 2000, 4000, 8000, 16000, -2 };  //-2 = motion sensor mode, -1 = regular mode, value >=0 = time in ms
unsigned char PRETTYBORDER_mode = 1;                                      //0 = 128*120 image, 1 = 128*114 image + 160*144 border, like the GB Camera
bool NIGHT_mode = 0;                                                      //0 = exp registers cap to 0xFFFF, 1 = clock hack. I'm honestly not super happy of the current version but it works
double motion_detection_threshold = 0.01;
double exposure_list[8] = { 0.5, 0.69, 0.79, 1, 1, 1.26, 1.44, 2 };  //list of exposures -1EV to +1EV by third roots of 2 steps for HDR mode
//double exposure_list[8]={ 1, 1, 1, 1, 1, 1, 1, 1 };  //for fancy multi-exposure images or signal to noise ratio increasing
//The dithering patterns from the single strategy mode
//See for details https://github.com/HerrZatacke/dither-pattern-gen/ and https://herrzatacke.github.io/dither-pattern-gen/
unsigned char Dithering_patterns_regular[] = { 0x2A, 0x5E, 0x9B, 0x51, 0x8B, 0xCA, 0x33, 0x69, 0xA6, 0x5A, 0x97, 0xD6, 0x44, 0x7C, 0xBA, 0x37, 0x6D, 0xAA, 0x4D, 0x87, 0xC6, 0x40, 0x78, 0xB6, 0x30, 0x65, 0xA2, 0x57, 0x93, 0xD2, 0x2D, 0x61, 0x9E, 0x54, 0x8F, 0xCE, 0x4A, 0x84, 0xC2, 0x3D, 0x74, 0xB2, 0x47, 0x80, 0xBE, 0x3A, 0x71, 0xAE };
bool GBCAMERA_mode = 1;  // 0 = single register strategy, 1 = Game Boy Camera strategy
//the two next parameters to set autocontrast in 8 bits mode in Game Boy Camera mode ONLY
unsigned char GB_v_min = 135;  //minimal voltage returned by the sensor in 8 bits DEC (1.5 volts is 112 but 135 matches the brightess with dithering patterns)
unsigned char GB_v_max = 195;  //maximal voltage returned by the sensor in 8 bits DEC (3.05 volts is 236 but 195 matches the brightess with dithering patterns)
bool BORDER_mode = 0;          //1 = enforce border enhancement whatever GBCAMERA_mode value
bool SMOOTH_mode = 0;          //1 = cancel border enhancement whatever GBCAMERA_mode value
//The dithering patterns from the Game Boy Camera, the "low" is only used for the highest exposure time (low light), because
unsigned char Dithering_patterns_low[] = { 0x8C, 0x98, 0xAC, 0x95, 0xA7, 0xDB, 0x8E, 0x9B, 0xB7, 0x97, 0xAA, 0xE7, 0x92, 0xA2, 0xCB, 0x8F, 0x9D, 0xBB, 0x94, 0xA5, 0xD7, 0x91, 0xA0, 0xC7, 0x8D, 0x9A, 0xB3, 0x96, 0xA9, 0xE3, 0x8C, 0x99, 0xAF, 0x95, 0xA8, 0xDF, 0x93, 0xA4, 0xD3, 0x90, 0x9F, 0xC3, 0x92, 0xA3, 0xCF, 0x8F, 0x9E, 0xBF };
unsigned char Dithering_patterns_high[] = { 0x89, 0x92, 0xA2, 0x8F, 0x9E, 0xC6, 0x8A, 0x95, 0xAB, 0x91, 0xA1, 0xCF, 0x8D, 0x9A, 0xBA, 0x8B, 0x96, 0xAE, 0x8F, 0x9D, 0xC3, 0x8C, 0x99, 0xB7, 0x8A, 0x94, 0xA8, 0x90, 0xA0, 0xCC, 0x89, 0x93, 0xA5, 0x90, 0x9F, 0xC9, 0x8E, 0x9C, 0xC0, 0x8C, 0x98, 0xB4, 0x8E, 0x9B, 0xBD, 0x8B, 0x97, 0xB1 };
bool FIXED_EXPOSURE_mode = 0;  //to activate fixed exposure delay mode
int FIXED_delay = 2048;        //here the result is a fixed exposure perfect for full moon photography
int FIXED_divider = 1;         //clock divider
//loockup table to convert 8 bits gray pixel value to RGB565 for the display
//Go to https://herrzatacke.github.io/gradient-values/ to generate the colorscale for json !
//Full project here https://github.com/HerrZatacke/gradient-values
//Default grayscale rendering in case json does not exist
unsigned short int lookup_TFT_RGB565[256] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0020, 0x0020, 0x0020, 0x0841, 0x0841, 0x0841, 0x0841, 0x0861, 0x0861, 0x0861, 0x0861,
                                              0x1082, 0x1082, 0x1082, 0x1082, 0x10A2, 0x10A2, 0x10A2, 0x10A2, 0x18C3, 0x18C3, 0x18C3, 0x18C3, 0x18E3, 0x18E3, 0x18E3, 0x18E3,
                                              0x2104, 0x2104, 0x2104, 0x2104, 0x2124, 0x2124, 0x2124, 0x2124, 0x2945, 0x2945, 0x2945, 0x2945, 0x2965, 0x2965, 0x2965, 0x2965,
                                              0x3186, 0x3186, 0x3186, 0x3186, 0x31A6, 0x31A6, 0x31A6, 0x31A6, 0x39C7, 0x39C7, 0x39C7, 0x39C7, 0x39E7, 0x39E7, 0x39E7, 0x39E7,
                                              0x4208, 0x4208, 0x4208, 0x4208, 0x4228, 0x4228, 0x4228, 0x4228, 0x4A49, 0x4A49, 0x4A49, 0x4A49, 0x4A69, 0x4A69, 0x4A69, 0x4A69,
                                              0x528A, 0x528A, 0x528A, 0x528A, 0x52AA, 0x52AA, 0x52AA, 0x52AA, 0x5ACB, 0x5ACB, 0x5ACB, 0x5ACB, 0x5AEB, 0x5AEB, 0x5AEB, 0x5AEB,
                                              0x630C, 0x630C, 0x630C, 0x630C, 0x632C, 0x632C, 0x632C, 0x632C, 0x6B4D, 0x6B4D, 0x6B4D, 0x6B4D, 0x6B6D, 0x6B6D, 0x6B6D, 0x6B6D,
                                              0x738E, 0x738E, 0x738E, 0x738E, 0x73AE, 0x73AE, 0x73AE, 0x73AE, 0x7BCF, 0x7BCF, 0x7BCF, 0x7BCF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF,
                                              0x8410, 0x8410, 0x8410, 0x8410, 0x8430, 0x8430, 0x8430, 0x8430, 0x8C51, 0x8C51, 0x8C51, 0x8C51, 0x8C71, 0x8C71, 0x8C71, 0x8C71,
                                              0x9492, 0x9492, 0x9492, 0x9492, 0x94B2, 0x94B2, 0x94B2, 0x94B2, 0x9CD3, 0x9CD3, 0x9CD3, 0x9CD3, 0x9CF3, 0x9CF3, 0x9CF3, 0x9CF3,
                                              0xA514, 0xA514, 0xA514, 0xA514, 0xA534, 0xA534, 0xA534, 0xA534, 0xAD55, 0xAD55, 0xAD55, 0xAD55, 0xAD75, 0xAD75, 0xAD75, 0xAD75,
                                              0xB596, 0xB596, 0xB596, 0xB596, 0xB5B6, 0xB5B6, 0xB5B6, 0xB5B6, 0xBDD7, 0xBDD7, 0xBDD7, 0xBDD7, 0xBDF7, 0xBDF7, 0xBDF7, 0xBDF7,
                                              0xC618, 0xC618, 0xC618, 0xC618, 0xC638, 0xC638, 0xC638, 0xC638, 0xCE59, 0xCE59, 0xCE59, 0xCE59, 0xCE79, 0xCE79, 0xCE79, 0xCE79,
                                              0xD69A, 0xD69A, 0xD69A, 0xD69A, 0xD6BA, 0xD6BA, 0xD6BA, 0xD6BA, 0xDEDB, 0xDEDB, 0xDEDB, 0xDEDB, 0xDEFB, 0xDEFB, 0xDEFB, 0xDEFB,
                                              0xE71C, 0xE71C, 0xE71C, 0xE71C, 0xE73C, 0xE73C, 0xE73C, 0xE73C, 0xEF5D, 0xEF5D, 0xEF5D, 0xEF5D, 0xEF7D, 0xEF7D, 0xEF7D, 0xEF7D,
                                              0xF79E, 0xF79E, 0xF79E, 0xF79E, 0xF7BE, 0xF7BE, 0xF7BE, 0xF7BE, 0xFFDF, 0xFFDF, 0xFFDF, 0xFFDF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };
unsigned char x_box = 8 * 8;         //x range for autoexposure (centered, like GB camera, 8 tiles)
unsigned char y_box = 7 * 8;         //y range for autoexposure (centered, like GB camera, 7 tiles)
bool FOCUS_mode = 0;                 //1 = Focus peaking mode overlayed on image
unsigned char FOCUS_threshold = 50;  //0..255, self explanatory
bool M64283FP = 0;                   //strategy for the M64283FP in single register strategy
//////////////end of default values in case config.json is not existing////////////////////////////////////////////////////////////////////

//////////////general parameters///////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char dithering_strategy[] = { 2, 1, 1, 1, 1, 0 };  //2 for single register strategy, 0 for Dithering_patterns_low, 1 Dithering_patterns_high, from high to low light
unsigned long delay_MOTION = 5000;                          //time to place the camera before motion detection starts
unsigned int max_files_per_folder = 1024;                   //self explanatory, in BMP mode
unsigned int low_exposure_limit = 0x0010;                   //absolute low exposure limit whatever the strategy

//to check if this is accurate with your compiling freq, the exposure time @FFFF must be closest as 1048 ms as possible and never less
#ifdef USE_OVERCLOCKING
unsigned int cycles = 22;  //nop clock setting for 250 MHz
#endif
#ifndef USE_OVERCLOCKING
unsigned int cycles = 11;  //nop clock setting for 133 MHz
#endif

unsigned char jittering_threshold = 13;                           //error threshold to keep/change registers in Game Boy Camera Mode
unsigned char Dithering_palette[4] = { 0x00, 0x55, 0xAA, 0xFF };  //colors as they will appear in the bmp file and display after dithering
unsigned char max_line_for_recording = 120;                       //choose 128 to record the whole image BUT powershell scripts have to be modified !!!
unsigned char max_line = 120;                                     //last 5-6 rows of pixels contains dark pixel value and various artifacts, so I remove 8 to have a full tile line
//unsigned char max_line = 128;                                   //outputs the whole image in "no border" mode
unsigned char x_min = (128 - x_box) / 2;       //calculate the autoexposure area limits
unsigned char y_min = (max_line - y_box) / 2;  //calculate the autoexposure area limits
unsigned char x_max = x_min + x_box;           //calculate the autoexposure area limits
unsigned char y_max = y_min + y_box;           //calculate the autoexposure area limits
unsigned char display_offset = 16;             //offset for image on the 128*160 display
unsigned char line_length = 4;                 //exposure area cross size
//Some various defines
#define NOP __asm__ __volatile__("nop\n\t")  //minimal possible delay
#define BITS_PER_PIXEL 16                    //How many bits per pixel in Sprite, here RGB565 format
//////////////end of general parameters////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////GPIO stuff///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LED 0  //to pi pico pin GPIO0 indicate exposure delay for the sensor <-> GND
#define RED 1  //to pi pico pin GPIO1 indicate recording to SD card of issue with SD card <-> GND
//TFT screens pins are quite flexible, I used a 1.8 TFT SPI 128*160 V1.1 model (ST7735 driver)
//pins are configured into the Bodmer TFT e_SPI library, DO NOT CHANGE HERE, see read.me for details
//Display SCK       to pi pico pin GPIO2
//Display SDA       to pi pico pin GPIO3
//Display CS        to pi pico pin GPIO4 (can use another pin if desired)
//Display AO        to pi pico pin GPIO5 (can use another pin if desired)
//Display RESET     to pi pico pin GPIO6 (can use another pin if desired)
//Display GND       to pi pico pin GND (0V)
//Display VCC       to pi pico pin VBUS Or +5V
//the following pins must be level shifted to 3.3V pico side<->5V sensor side
#define READ 7    //to pi pico pin GPIO7 Read image signal, goes high on rising clock
#define CLOCK 8   //to pi pico pin GPIO8 Clock input, pulled down internally, no specification given for frequency
#define RESET 9   //to pi pico pin GPIO9 system reset, pulled down internally, active low, sent on rising edge of clock
#define LOAD 10   //to pi pico pin GPIO10 parameter set enable, pulled down internally, Raise high as you clear the last bit of each register you send
#define SIN 11    //to pi pico pin GPIO11 Image sensing start, pulled down internally, has to be high before rising CLOCK
#define START 12  //to pi pico pin GPIO12 Image sensing start, pulled down internally, has to be high before rising CLOCK
//next pins are I/O for buttons
#define PUSH 13  //to pi pico pin GPIO13 action button <-> 3.3V - action button to record
#define TLC 14   //to pi pico pin GPIO14 <-> 3.3V - timelapse<->regular camera mode
#define LOCK 15  //to pi pico pin GPIO15  <-> 3.3V - Lock exposure
//Beware, SD card MUST be attached to these pins as the pico seems not very tolerant with SD card pinout, they CANNOT be changed
//SD_MISO - to pi pico pin GPIO16
//SD_CS   - to pi pico pin GPIO17
#define CHIPSELECT 17  //for SD card, can be moved but I recommend not changing it either
//SD_SCK  - to pi pico pin GPIO18
//SD_MOSI - to pi pico pin GPIO19
#define HDR 20     //to pi pico pin GPIO20 <-> 3.3V - HDR mode
#define DITHER 21  //to pi pico pin GPIO21 <-> 3.3V - dithering with Bayer matrix

//GPIO 22 is reserved for TADD register (see next), but can be routed to external trigger if you need one
//if used to trigger TADD pin, better use a free level shifter channel rather in order to avoid using 3.3 volts on the sensor
//#define INOUT 22  //to trigger input/output in 3.3 volts, see dedicated pins on the PCB
#ifdef TADDREGISTER
#define TADD 22  //to pin TADD of the M64283FP CMOS sensor
////////////////////////////START2x4bits/STOP2x4bits/see datasheet
///////////////////////////{ 0bXXXXYYYY, 0bXXXXYYYY };
unsigned char camTADD[2] = { 0b00000000, 0b11111111 };  //additional registers 8 and 9, START and STOP of image area in tiles (16x16)
//these parameters should produce a 128*128 image too but never tested...
#endif

//GPIO 23 and 24 are reserved for internal use of the pi pico (on-board SMPS Power Save and VBUS sense respectively), usable but not recommended
//seems that using one of them could help ADC stability but I never had any issue with them...
#define INT 25   //to pi pico pin GPIO25 internal LED;
#define VOUT 26  //to pi pico pin GPIO26/A0 Analog signal from sensor, read shortly after clock is set low, native 12 bits, converted to 8 bits
//GPIO 27 free ADC or digital channels
//GPIO 28 free ADC or digital channels
//GPIO 29 is reserved to measure VSYS
//It is advised to attach pi pico pin RUN pin to any GND via a pushbutton for resetting the pico
//////////////end of GPIO stuff///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////sensor stuff////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//some real settings used by the Mitsubishi M64282FP sensor on Game Boy Camera, except exposure
//reg0 = 0b10011111; % Z1 Z0 O5 O4 O3 O2 O1 O0 enable black level calibration with fixed reg O/ output reference voltage fine -> O and V add themselves, if !V==0 (V = 0 is not allowed anyway)
//reg1 = 0b11100100; % N VH1 VH0 G4 G3 G2 G1 G0 set edge / type of edge / gain
//reg2 = 0b00000001; % C17 C16 C15 C14 C13 C12 C11 C10 / exposure time by 4096 ms steps (max 1.0486 s)
//reg3 = 0b00000000; % C07 C06 C05 C04 C03 C02 C01 C00 / exposure time by 16 µs steps (max 4096 ms)
//reg4 = 0b00000001; % P7 P6 P5 P4 P3 P2 P1 P0 filtering kernels, always 0x01 on a GB camera, but can be different here
//reg5 = 0b00000000; % M7 M6 M5 M4 M3 M2 M1 M0 filtering kernels, always 0x00 on a GB camera, but can be different here
//reg6 = 0b00000001; % X7 X6 X5 X4 X3 X2 X1 X0 filtering kernels, always 0x01 on a GB camera, but can be different here
//reg7 = 0b00000011; % E3 E2 E1 E0 I V2 V1 V0 Edge enhancement ratio / invert / Output node bias voltage raw -> O and V add themselves, if !V==0 (forbidden state)
//P, M and X registers allows pure edge extraction for example, see datasheet, all other registers must be modified accordingly
//Game Boy Camera strategy: uses half the voltage scale
//Note for finicky devs: register O must theoretically be adjusted between different sensors to get seamless transitions...
///////////////////////////{ 0bZZOOOOOO, 0bNVVGGGGG, 0bCCCCCCCC, 0bCCCCCCCC, 0bPPPPPPPP, 0bMMMMMMMM, 0bXXXXXXXX, 0bEEEEIVVV};
unsigned char camReg1[8] = { 0b10101001, 0b00100000, 0b00000000, 0b00000000, 0b00000001, 0b00000000, 0b00000001, 0b00000011 };  //low exposure time - high light
//transition@ C=0x0030
unsigned char camReg2[8] = { 0b10101001, 0b11100000, 0b00000000, 0b00000000, 0b00000001, 0b00000000, 0b00000001, 0b00000011 };
//transition@ C=0x0D80
unsigned char camReg3[8] = { 0b10101011, 0b11100100, 0b00000000, 0b00000000, 0b00000001, 0b00000000, 0b00000001, 0b00000011 };
//transition@ C=0x3500
unsigned char camReg4[8] = { 0b10101111, 0b11101000, 0b00000000, 0b00000000, 0b00000001, 0b00000000, 0b00000001, 0b00000011 };
//transition@ C=0x8500
unsigned char camReg5[8] = { 0b10100111, 0b00001010, 0b00000000, 0b00000000, 0b00000001, 0b00000000, 0b00000001, 0b00000011 };  //high exposure time - low light
//the voltage scale in this mode is defined earlier in config.h

//DashBoy Camera strategy for the M64282FP sensor: uses the maximum voltage scale
///////////////////////////////////{ 0bZZOOOOOO, 0bNVVGGGGG, 0bCCCCCCCC, 0bCCCCCCCC, 0bPPPPPPPP, 0bMMMMMMMM, 0bXXXXXXXX, 0bEEEEIVVV };
unsigned char camReg_M64282FP[8] = { 0b10000000, 0b11100111, 0b00010000, 0b00000000, 0b00000001, 0b00000000, 0b00000001, 0b00000001 };  //registers

//these values are not embedded into the config.json but could
unsigned char M64282FP_v_min = 50;   //0 is OV, 255 is 3.3 volts
unsigned char M64282FP_v_max = 180;  //0 is OV, 255 is 3.3 volts

//DashBoy Camera strategy for the unobtainium M64283FP sensor
//detail of the registers
//reg0 = like M64282FP
//reg1 = like M64282FP
//reg2 = like M64282FP
//reg3 = like M64282FP
//reg4 = SH  AZ  CL  []  [P3  P2  P1  P0]
//reg5 = PX  PY  MV4 OB  [M3  M2  M1  M0] //OB low enables outputing black level on line 1 instead of VOUT
//reg6 = MV3 MV2 MV1 MV0 [X3  X2  X1  X0]
//reg7 = like M64282FP but E register is shifted in value
//reg8 = ST7  ST6  ST5  ST4  ST3  ST2  ST1  ST0  - random access start address by (x, y), beware image divided into 8x8 tiles !
//reg9 = END7 END6 END5 END4 END3 END2 END1 END0 - random access stop address by (x', y'), beware image divided into 8x8 tiles !
///////////////////////////////////{ 0bZZOOOOOO, 0bNVVGGGGG, 0bCCCCCCCC, 0bCCCCCCCC, 0bSAC_PPPP, 0bPPMOMMMM, 0bXXXXXXXX, 0bEEEEIVVV };
unsigned char camReg_M64283FP[8] = { 0b10000000, 0b11100111, 0b00010000, 0b00000000, 0b00000001, 0b00000000, 0b00000001, 0b01000001 };  //registers with black level calibration
//unsigned char camReg_M64283FP[8] = { 0b10000000, 0b11100111, 0b00010000, 0b00000000, 0b11000001, 0b00010000, 0b00000001, 0b01000001 };  //registers without black level calibration, image is crap ^^
//2D edge enhancement activated + set gain to 24.5dB
//CL + AZ + SH + OB = LOW -> to what I understand, outputs black level on the first pixel line and activate the auto-calibration circuit
//AZ + SH + OB = HIGH -> acts as a M64282FP (whatever CL state ? not written in datasheet) -> the image becomes disgusting !
//So to have a real 128x128 image, AZ + SH + OB must be HIGH
//the datasheet says exactly "Adjust the O register so that this dark pixel output (optical black) level becomes the Vref value"
//this is valid both for the 82FP and 83FP, except that it is automatic with the 83FP and open loop with the 82FP

//these next values are not embedded into the config.json but could
unsigned char M64283FP_v_min = 60;   //0 is OV, 255 is 3.3 volts
unsigned char M64283FP_v_max = 190;  //0 is OV, 255 is 3.3 volts
//////////////end of sensor stuff////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//https://www.calculator.net/hex-calculator.html is your friend
//https://www.rapidtables.com/convert/number/hex-to-decimal.html too
unsigned char BMP_header_generic[54] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

//BMP indexed palette for 8 bits RGB
const unsigned char BMP_indexed_palette[1024] = { 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02,
                                                  0x02, 0x00, 0x03, 0x03, 0x03, 0x00, 0x04, 0x04, 0x04, 0x00, 0x05, 0x05, 0x05, 0x00, 0x06, 0x06,
                                                  0x06, 0x00, 0x07, 0x07, 0x07, 0x00, 0x08, 0x08, 0x08, 0x00, 0x09, 0x09, 0x09, 0x00, 0x0A, 0x0A,
                                                  0x0A, 0x00, 0x0B, 0x0B, 0x0B, 0x00, 0x0C, 0x0C, 0x0C, 0x00, 0x0D, 0x0D, 0x0D, 0x00, 0x0E, 0x0E,
                                                  0x0E, 0x00, 0x0F, 0x0F, 0x0F, 0x00, 0x10, 0x10, 0x10, 0x00, 0x11, 0x11, 0x11, 0x00, 0x12, 0x12,
                                                  0x12, 0x00, 0x13, 0x13, 0x13, 0x00, 0x14, 0x14, 0x14, 0x00, 0x15, 0x15, 0x15, 0x00, 0x16, 0x16,
                                                  0x16, 0x00, 0x17, 0x17, 0x17, 0x00, 0x18, 0x18, 0x18, 0x00, 0x19, 0x19, 0x19, 0x00, 0x1A, 0x1A,
                                                  0x1A, 0x00, 0x1B, 0x1B, 0x1B, 0x00, 0x1C, 0x1C, 0x1C, 0x00, 0x1D, 0x1D, 0x1D, 0x00, 0x1E, 0x1E,
                                                  0x1E, 0x00, 0x1F, 0x1F, 0x1F, 0x00, 0x20, 0x20, 0x20, 0x00, 0x21, 0x21, 0x21, 0x00, 0x22, 0x22,
                                                  0x22, 0x00, 0x23, 0x23, 0x23, 0x00, 0x24, 0x24, 0x24, 0x00, 0x25, 0x25, 0x25, 0x00, 0x26, 0x26,
                                                  0x26, 0x00, 0x27, 0x27, 0x27, 0x00, 0x28, 0x28, 0x28, 0x00, 0x29, 0x29, 0x29, 0x00, 0x2A, 0x2A,
                                                  0x2A, 0x00, 0x2B, 0x2B, 0x2B, 0x00, 0x2C, 0x2C, 0x2C, 0x00, 0x2D, 0x2D, 0x2D, 0x00, 0x2E, 0x2E,
                                                  0x2E, 0x00, 0x2F, 0x2F, 0x2F, 0x00, 0x30, 0x30, 0x30, 0x00, 0x31, 0x31, 0x31, 0x00, 0x32, 0x32,
                                                  0x32, 0x00, 0x33, 0x33, 0x33, 0x00, 0x34, 0x34, 0x34, 0x00, 0x35, 0x35, 0x35, 0x00, 0x36, 0x36,
                                                  0x36, 0x00, 0x37, 0x37, 0x37, 0x00, 0x38, 0x38, 0x38, 0x00, 0x39, 0x39, 0x39, 0x00, 0x3A, 0x3A,
                                                  0x3A, 0x00, 0x3B, 0x3B, 0x3B, 0x00, 0x3C, 0x3C, 0x3C, 0x00, 0x3D, 0x3D, 0x3D, 0x00, 0x3E, 0x3E,
                                                  0x3E, 0x00, 0x3F, 0x3F, 0x3F, 0x00, 0x40, 0x40, 0x40, 0x00, 0x41, 0x41, 0x41, 0x00, 0x42, 0x42,
                                                  0x42, 0x00, 0x43, 0x43, 0x43, 0x00, 0x44, 0x44, 0x44, 0x00, 0x45, 0x45, 0x45, 0x00, 0x46, 0x46,
                                                  0x46, 0x00, 0x47, 0x47, 0x47, 0x00, 0x48, 0x48, 0x48, 0x00, 0x49, 0x49, 0x49, 0x00, 0x4A, 0x4A,
                                                  0x4A, 0x00, 0x4B, 0x4B, 0x4B, 0x00, 0x4C, 0x4C, 0x4C, 0x00, 0x4D, 0x4D, 0x4D, 0x00, 0x4E, 0x4E,
                                                  0x4E, 0x00, 0x4F, 0x4F, 0x4F, 0x00, 0x50, 0x50, 0x50, 0x00, 0x51, 0x51, 0x51, 0x00, 0x52, 0x52,
                                                  0x52, 0x00, 0x53, 0x53, 0x53, 0x00, 0x54, 0x54, 0x54, 0x00, 0x55, 0x55, 0x55, 0x00, 0x56, 0x56,
                                                  0x56, 0x00, 0x57, 0x57, 0x57, 0x00, 0x58, 0x58, 0x58, 0x00, 0x59, 0x59, 0x59, 0x00, 0x5A, 0x5A,
                                                  0x5A, 0x00, 0x5B, 0x5B, 0x5B, 0x00, 0x5C, 0x5C, 0x5C, 0x00, 0x5D, 0x5D, 0x5D, 0x00, 0x5E, 0x5E,
                                                  0x5E, 0x00, 0x5F, 0x5F, 0x5F, 0x00, 0x60, 0x60, 0x60, 0x00, 0x61, 0x61, 0x61, 0x00, 0x62, 0x62,
                                                  0x62, 0x00, 0x63, 0x63, 0x63, 0x00, 0x64, 0x64, 0x64, 0x00, 0x65, 0x65, 0x65, 0x00, 0x66, 0x66,
                                                  0x66, 0x00, 0x67, 0x67, 0x67, 0x00, 0x68, 0x68, 0x68, 0x00, 0x69, 0x69, 0x69, 0x00, 0x6A, 0x6A,
                                                  0x6A, 0x00, 0x6B, 0x6B, 0x6B, 0x00, 0x6C, 0x6C, 0x6C, 0x00, 0x6D, 0x6D, 0x6D, 0x00, 0x6E, 0x6E,
                                                  0x6E, 0x00, 0x6F, 0x6F, 0x6F, 0x00, 0x70, 0x70, 0x70, 0x00, 0x71, 0x71, 0x71, 0x00, 0x72, 0x72,
                                                  0x72, 0x00, 0x73, 0x73, 0x73, 0x00, 0x74, 0x74, 0x74, 0x00, 0x75, 0x75, 0x75, 0x00, 0x76, 0x76,
                                                  0x76, 0x00, 0x77, 0x77, 0x77, 0x00, 0x78, 0x78, 0x78, 0x00, 0x79, 0x79, 0x79, 0x00, 0x7A, 0x7A,
                                                  0x7A, 0x00, 0x7B, 0x7B, 0x7B, 0x00, 0x7C, 0x7C, 0x7C, 0x00, 0x7D, 0x7D, 0x7D, 0x00, 0x7E, 0x7E,
                                                  0x7E, 0x00, 0x7F, 0x7F, 0x7F, 0x00, 0x80, 0x80, 0x80, 0x00, 0x81, 0x81, 0x81, 0x00, 0x82, 0x82,
                                                  0x82, 0x00, 0x83, 0x83, 0x83, 0x00, 0x84, 0x84, 0x84, 0x00, 0x85, 0x85, 0x85, 0x00, 0x86, 0x86,
                                                  0x86, 0x00, 0x87, 0x87, 0x87, 0x00, 0x88, 0x88, 0x88, 0x00, 0x89, 0x89, 0x89, 0x00, 0x8A, 0x8A,
                                                  0x8A, 0x00, 0x8B, 0x8B, 0x8B, 0x00, 0x8C, 0x8C, 0x8C, 0x00, 0x8D, 0x8D, 0x8D, 0x00, 0x8E, 0x8E,
                                                  0x8E, 0x00, 0x8F, 0x8F, 0x8F, 0x00, 0x90, 0x90, 0x90, 0x00, 0x91, 0x91, 0x91, 0x00, 0x92, 0x92,
                                                  0x92, 0x00, 0x93, 0x93, 0x93, 0x00, 0x94, 0x94, 0x94, 0x00, 0x95, 0x95, 0x95, 0x00, 0x96, 0x96,
                                                  0x96, 0x00, 0x97, 0x97, 0x97, 0x00, 0x98, 0x98, 0x98, 0x00, 0x99, 0x99, 0x99, 0x00, 0x9A, 0x9A,
                                                  0x9A, 0x00, 0x9B, 0x9B, 0x9B, 0x00, 0x9C, 0x9C, 0x9C, 0x00, 0x9D, 0x9D, 0x9D, 0x00, 0x9E, 0x9E,
                                                  0x9E, 0x00, 0x9F, 0x9F, 0x9F, 0x00, 0xA0, 0xA0, 0xA0, 0x00, 0xA1, 0xA1, 0xA1, 0x00, 0xA2, 0xA2,
                                                  0xA2, 0x00, 0xA3, 0xA3, 0xA3, 0x00, 0xA4, 0xA4, 0xA4, 0x00, 0xA5, 0xA5, 0xA5, 0x00, 0xA6, 0xA6,
                                                  0xA6, 0x00, 0xA7, 0xA7, 0xA7, 0x00, 0xA8, 0xA8, 0xA8, 0x00, 0xA9, 0xA9, 0xA9, 0x00, 0xAA, 0xAA,
                                                  0xAA, 0x00, 0xAB, 0xAB, 0xAB, 0x00, 0xAC, 0xAC, 0xAC, 0x00, 0xAD, 0xAD, 0xAD, 0x00, 0xAE, 0xAE,
                                                  0xAE, 0x00, 0xAF, 0xAF, 0xAF, 0x00, 0xB0, 0xB0, 0xB0, 0x00, 0xB1, 0xB1, 0xB1, 0x00, 0xB2, 0xB2,
                                                  0xB2, 0x00, 0xB3, 0xB3, 0xB3, 0x00, 0xB4, 0xB4, 0xB4, 0x00, 0xB5, 0xB5, 0xB5, 0x00, 0xB6, 0xB6,
                                                  0xB6, 0x00, 0xB7, 0xB7, 0xB7, 0x00, 0xB8, 0xB8, 0xB8, 0x00, 0xB9, 0xB9, 0xB9, 0x00, 0xBA, 0xBA,
                                                  0xBA, 0x00, 0xBB, 0xBB, 0xBB, 0x00, 0xBC, 0xBC, 0xBC, 0x00, 0xBD, 0xBD, 0xBD, 0x00, 0xBE, 0xBE,
                                                  0xBE, 0x00, 0xBF, 0xBF, 0xBF, 0x00, 0xC0, 0xC0, 0xC0, 0x00, 0xC1, 0xC1, 0xC1, 0x00, 0xC2, 0xC2,
                                                  0xC2, 0x00, 0xC3, 0xC3, 0xC3, 0x00, 0xC4, 0xC4, 0xC4, 0x00, 0xC5, 0xC5, 0xC5, 0x00, 0xC6, 0xC6,
                                                  0xC6, 0x00, 0xC7, 0xC7, 0xC7, 0x00, 0xC8, 0xC8, 0xC8, 0x00, 0xC9, 0xC9, 0xC9, 0x00, 0xCA, 0xCA,
                                                  0xCA, 0x00, 0xCB, 0xCB, 0xCB, 0x00, 0xCC, 0xCC, 0xCC, 0x00, 0xCD, 0xCD, 0xCD, 0x00, 0xCE, 0xCE,
                                                  0xCE, 0x00, 0xCF, 0xCF, 0xCF, 0x00, 0xD0, 0xD0, 0xD0, 0x00, 0xD1, 0xD1, 0xD1, 0x00, 0xD2, 0xD2,
                                                  0xD2, 0x00, 0xD3, 0xD3, 0xD3, 0x00, 0xD4, 0xD4, 0xD4, 0x00, 0xD5, 0xD5, 0xD5, 0x00, 0xD6, 0xD6,
                                                  0xD6, 0x00, 0xD7, 0xD7, 0xD7, 0x00, 0xD8, 0xD8, 0xD8, 0x00, 0xD9, 0xD9, 0xD9, 0x00, 0xDA, 0xDA,
                                                  0xDA, 0x00, 0xDB, 0xDB, 0xDB, 0x00, 0xDC, 0xDC, 0xDC, 0x00, 0xDD, 0xDD, 0xDD, 0x00, 0xDE, 0xDE,
                                                  0xDE, 0x00, 0xDF, 0xDF, 0xDF, 0x00, 0xE0, 0xE0, 0xE0, 0x00, 0xE1, 0xE1, 0xE1, 0x00, 0xE2, 0xE2,
                                                  0xE2, 0x00, 0xE3, 0xE3, 0xE3, 0x00, 0xE4, 0xE4, 0xE4, 0x00, 0xE5, 0xE5, 0xE5, 0x00, 0xE6, 0xE6,
                                                  0xE6, 0x00, 0xE7, 0xE7, 0xE7, 0x00, 0xE8, 0xE8, 0xE8, 0x00, 0xE9, 0xE9, 0xE9, 0x00, 0xEA, 0xEA,
                                                  0xEA, 0x00, 0xEB, 0xEB, 0xEB, 0x00, 0xEC, 0xEC, 0xEC, 0x00, 0xED, 0xED, 0xED, 0x00, 0xEE, 0xEE,
                                                  0xEE, 0x00, 0xEF, 0xEF, 0xEF, 0x00, 0xF0, 0xF0, 0xF0, 0x00, 0xF1, 0xF1, 0xF1, 0x00, 0xF2, 0xF2,
                                                  0xF2, 0x00, 0xF3, 0xF3, 0xF3, 0x00, 0xF4, 0xF4, 0xF4, 0x00, 0xF5, 0xF5, 0xF5, 0x00, 0xF6, 0xF6,
                                                  0xF6, 0x00, 0xF7, 0xF7, 0xF7, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0xF9, 0xF9, 0xF9, 0x00, 0xFA, 0xFA,
                                                  0xFA, 0x00, 0xFB, 0xFB, 0xFB, 0x00, 0xFC, 0xFC, 0xFC, 0x00, 0xFD, 0xFD, 0xFD, 0x00, 0xFE, 0xFE,
                                                  0xFE, 0x00, 0xFF, 0xFF, 0xFF, 0x00 };

//HEX characters in ASCII table for serial output to Matlab
const unsigned char LUT_serial[16] = { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46 };

//a bit of explanations on data structure is necessary here:
//CamData[128 * 128] contains the raw signal from sensor in 8 bits
//HDRData[128 * 128] contains an average of raw signal data from sensor with increased dynamic range
//lookup_serial[CamData[...]] is the sensor data with autocontrast in 8 bits for storing to SD card or sending to serial
//lookup_TFT_RGB565[lookup_serial[CamData[...]]] is the sensor data with autocontrast in 16 bits RGB565 for the display
//BayerData[128 * 128] contains the sensor data with dithering AND autocontrast in 2 bpp but stored in 8 bits
//lookup_TFT_RGB565[BayerData[...]] contains the sensor data with dithering AND autocontrast in 16 bits RGB565 for the display
//and so on, you get the idea...

//I use 1D array only, because I was not sure wether 2D arrays would be supported. Anyway, It changes nothing
//////////////global variables////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char Dithering_patterns[48];       //storage for dithering tables
unsigned char lookup_serial[256];           //autocontrast table generated in setup() from v_min and v_max
unsigned char lookup_pico_to_GBD[256];      //convert 0<->3.3V scale from pico to 0<->3.0V scale from MAC-GBD
unsigned char CamData[128 * 128];           //sensor data in 8 bits per pixel
unsigned char CamData_previous[128 * 128];  //sensor data in 8 bits per pixel from preceding loop for motion detection
unsigned char EdgeData[128 * 128];          //edge detection data in 8 bits per pixel
unsigned char BmpData[128 * 128];           //sensor data with autocontrast ready to be merged with BMP header
unsigned char BigBmpData[160 * 144];        //sensor data with autocontrast and pretty border ready to be merged with BMP header
unsigned short int HDRData[128 * 128];      //cumulative data for HDR imaging -1EV, +1EV + 2xOEV, 4 images in total
unsigned char Bayer_matW_LG[4 * 4];         //Bayer matrix to apply dithering for each image pixel white to light gray
unsigned char Bayer_matLG_DG[4 * 4];        //Bayer matrix to apply dithering for each image pixel light gray to dark gray
unsigned char Bayer_matDG_B[4 * 4];         //Bayer matrix to apply dithering for each image pixel dark gray to dark
unsigned char BayerData[128 * 128];         //dithered image data
unsigned char camReg[8];                    //empty register array
unsigned char camReg_storage[8];            //empty register array
unsigned int clock_divider = 1;             //time delay in processor cycles to cheat the exposure of the sensor
unsigned short int pixel_TFT_RGB565;
unsigned long currentTime = 0;
unsigned long previousTime = 0;
unsigned long currentTime_MOTION = 0;
unsigned long currentTime_exp = 0;
unsigned long previousTime_exp = 0;
unsigned long file_number;
unsigned long Next_ID, Next_dir;       //for directories and filenames
unsigned long TIMELAPSE_deadtime = 0;  //to introduce a deadtime for timelapses in ms, is read from config.json
unsigned long masked_pixels = 0;       //accumulator for dark/masked pixels
unsigned int current_exposure, new_exposure;
unsigned int low_exposure_threshold = 0;
unsigned int files_on_folder = 0;
unsigned int MOTION_sensor_counter = 0;
int dark_level = 0;  //for DEGABAME mode
int V_ref = 0;       //for DEGABAME mode
int O_reg = 0;       //for DEGABAME mode
int V_Offset = 0;    //for DEGABAME mode
unsigned char v_min, v_max;
double difference = 0;
double exposure_error = 0;
double mean_value = 0;
double error = 0;
double difference_threshold;  //trigger threshold for motion sensor
double multiplier = 1;        //deals with autoexposure algorithm
bool TIMELAPSE_mode = 0;      //0 = use s a regular camera, 1 = recorder for timelapses
bool HDR_mode = 0;            //0 = regular capture, 1 = HDR mode
bool DITHER_mode = 0;         //0 = Dithering ON, 0 = dithering OFF
bool image_TOKEN = 0;         //reserved for CAMERA mode
bool recording = 0;           //0 = idle mode, 1 = recording mode
bool sensor_READY = 0;        //reserved, for bug on sensor
bool SDcard_READY = 0;        //reserved, for bug on SD
bool JSON_ready = 0;          //reserved, for bug on config.txt
bool LOCK_exposure = 0;       //reserved, for locking exposure
bool MOTION_sensor = 0;       //reserved, to trigger motion sensor mode
bool overshooting = 0;        //reserved, for register anti-jittering system

char storage_file_name[20], storage_file_dir[20], storage_deadtime[20], exposure_string[20], multiplier_string[20];
char error_string[20], remaining_deadtime[20], exposure_string_ms[20], files_on_folder_string[20], register_string[2], difference_string[8];
char mask_pixels_string[20];
char num_HDR_images = sizeof(exposure_list) / sizeof(double);   //get the HDR or multi-exposure list size
char num_timelapses = sizeof(timelapse_list) / sizeof(double);  //get the timelapse list size
char rank_timelapse = 0;                                        //rank in the timelapse array
char register_strategy = 0;                                     //strategy # of the Game Boy Camera emulation
//////////////end of global variables///////////////////////////////////////////////////////////////////////////////////////////////////////////////

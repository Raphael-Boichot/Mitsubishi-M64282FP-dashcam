//By Raphaël BOICHOT, made around 2023-04-27
//this is an autonomous recorder for the Mitsubishi M64282FP artificial retina of the Game Boy Camera
//from a code of Laurent Saint-Marcel (lstmarcel@yahoo.fr) written in 2005/07/05
//stole some code for Rafael Zenaro NeoGB printer: https://github.com/zenaro147/NeoGB-Printer
//version for Arduino here (requires a computer): https://github.com/Raphael-Boichot/Play-with-the-Game-Boy-Camera-Mitsubishi-M64282FP-sensor
//Made to be compiled on the Arduino IDE, using these libraries:
//https://github.com/earlephilhower/arduino-pico core for pi-pico
//https://github.com/Bodmer/TFT_eSPI library for TFT display
//Beware, I'm not a C developper at all so it won't be a pretty code !

#include "hardware/adc.h" //the GPIO commands are here

#define NOP __asm__ __volatile__ ("nop\n\t") //// minimal possible delay

#include <TFT_eSPI.h>                 // Include the graphics library (this includes the sprite functions)
TFT_eSPI    tft = TFT_eSPI();         // Create object "tft"
TFT_eSprite img = TFT_eSprite(&tft);  // Create Sprite object "img" with pointer to "tft" object
// the pointer is used by pushSprite() to push it onto the TFT
#define BITS_PER_PIXEL 16             // How many bits per pixel in Sprite, here RGB565 format

#include <SPI.h>  //for SD
#include <SD.h>  //for SD

//BMP header for a 128x120 8 bit grayscale image (RGB format, non indexed). Height (120) is inverted to have the image in correct orientation
const unsigned char BMP_header [] PROGMEM = {0x42, 0x4D, 0xB6, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x04, 0x00, 0x00, 0x28, 0x00,
                                             0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x88, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00,
                                             0x00, 0x00, 0x80, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02,
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
                                             0xFE, 0x00, 0xFF, 0xFF, 0xFF, 0x00
                                            };

// the order of pins has no importance except that VOUT must be on some ADC
const uint VOUT =  26; //to pi pico pin GPIO26/A0 Analog signal from sensor, read shortly after clock is set low, native 12 bits, converted to 8 bits

//the following pins must be shifted to 3.3V pico side<->5V sensor side
const uint READ =  7;   //to pi pico pin GPIO7 Read image signal, goes high on rising clock
const uint CLOCK = 8;   //to pi pico pin GPIO8 Clock input, pulled down internally, no specification given for frequency
const uint RESET = 9;   //to pi pico pin GPIO9 system reset, pulled down internally, active low, sent on rising edge of clock
const uint LOAD =  10;  //to pi pico pin GPIO10 parameter set enable, pulled down internally, Raise high as you clear the last bit of each register you send
const uint SIN =   11;  //to pi pico pin GPIO11 Image sensing start, pulled down internally, has to be high before rising CLOCK
const uint START = 12;  //to pi pico pin GPIO12 Image sensing start, pulled down internally, has to be high before rising CLOCK

//the following are intended to allow interfering with the device
const uint LED =   15; //to pi pico pin GPIO15 indicate exposure delay for the sensor <-> GND
const uint RED =   14; //to pi pico pin GPIO14 indicate recording to SD card of issue with SD card <-> GND
const uint PUSH =  13; //to pi pico pin GPIO13 action button <-> 3.3V
const uint HDR =   20; //to pi pico pin GPIO20 <-> 3.3V
const uint BORDER = 22; //to pi pico pin GPIO22 <-> 3.3V
// it is advised to attach pi pico pin RUN pin to any GND via a pushbutton for resetting the pico

//Beware, SD card MUST be attached to these pins as the pico seems not very tolerant with SD card pinout, they cannot be changed
//   SD_MISO - to pi pico pin GPIO16
//   SD_MOSI - to pi pico pin GPIO19
//   SD_CS   - to pi pico pin GPIO17
//   SD_SCK  - to pi pico pin GPIO18
const uint chipSelect = 17;//for SD card, but I recommend not changing it either

//TFT screens pins are more flexible, I used a 1.8 TFT SPI 128*160 V1.1 model (ST7735 driver)
// pins are configured into the Bodmer TFT e_SPI library, not here, see read.me for details
// Display LED       to pi pico pin 3V3
// Display SCK       to pi pico pin GPIO2
// Display SDA       to pi pico pin GPIO3
// Display CS        to pi pico pin GPIO4 (can use another pin if desired)
// Display AO        to pi pico pin GPIO5 (can use another pin if desired)
// Display RESET     to pi pico pin GPIO6 (can use another pin if desired)
// Display GND       to pi pico pin GND (0V)
// Display VCC       to pi pico pin VBUS Or +5V

//With these registers, the output voltage is between 0.14 and 3.04 volts (on 3.3 volts).
//the ADC resolution is 0.8 mV (3.3/2^12, 12 bits) cut to 12.9 mV (8 bits)
//registers are close of those from the Game Boy Camera in mid light
unsigned char camReg[8] = {0b10011111, 0b11101000, 0b00000001, 0b00000000, 0b00000001, 0b000000000, 0b00000001, 0b00000011}; //registers
//registers with no border enhancement (gives very soft image)
//unsigned char camReg[8] = {0b10011111, 0b00001000, 0b00000001, 0b00000000, 0b00000001, 0b000000000, 0b00000001, 0b00000011}; //registers

unsigned int lookup_TFT_RGB565[256];//loockup table to convert gray pixel value to RGB565 for the display
unsigned char lookup_serial[256];//loockup table for serial output
const unsigned char LUT_serial[16] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46}; //HEX characters in ASCII table
//lookup table to read config.txt
const unsigned char ASCII_to_num[256] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned char reg, char1, char2;
unsigned char CamData[128 * 128];// sensor data in 8 bits per pixel
unsigned char BmpData[128 * 128];// sensor data with autocontrast ready to be merged with BMP header
unsigned int HDRData[128 * 128];// cumulative data for HDR imaging -1EV, +1EV + 2xOEV, 4 images in total
const int cycles = 12; //time delay in processor cycles, to fit with the 1MHz advised clock cycle for the sensor (set with a datalogger, do not touch !)
const unsigned char v_min = 11; //minimal voltage returned by the sensor in 8 bits DEC
const unsigned char v_max = 236;//maximal voltage returned by the sensor in 8 bits DEC
const unsigned int debouncing_delay = 500; //debouncing delay for pushbutton
unsigned long currentTime = 0;
unsigned long previousTime = 0;
unsigned long deadtime = 2000; //to introduce a deadtime for timelapses in ms. Default is 100 ms to avoid SD card death by chocking, is read from config.txt
unsigned long Next_ID, Next_dir;//for directories and filenames
unsigned long file_number;
unsigned int current_exposure;
bool recording = 0;//0 = idle mode, 1 = recording mode
bool HDR_mode = 0; //0 = regular capture, 1 = HDR mode
bool BORDER_mode = 1; //1 = border enhancement ON, 0 = border enhancement OFF
char storage_file_name[20];
char storage_file_dir[20];
char storage_deadtime[20];
char exposure_string[20];

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
  img.setTextSize(1);
  img.setTextColor(TFT_WHITE);
  img.setCursor(0, 0);
  img.println(F("Initializing SD card..."));

  //see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    img.setCursor(0, 8);
    img.setTextColor(TFT_RED);
    img.println(F("Card failure !!!"));
  }
  else {
    img.setTextColor(TFT_GREEN);
    img.setCursor(0, 8);
    img.println(F("Card mounted"));
  }
  //
  deadtime = get_dead_time("/Delay.txt", deadtime);//get the dead time for timelapse from config.txt
  sprintf(storage_deadtime, "Delay: %d ms", deadtime); //concatenate string for display
  img.setTextColor(TFT_BLUE);
  img.setCursor(0, 16);
  img.println(F(storage_deadtime));
  img.setTextColor(TFT_ORANGE);
  img.setCursor(0, 24);
  img.println(F("Preset exposure..."));
  img.pushSprite(0, 0);

  Serial.begin(2000000);
  ID_file_creator("/ID_storage.bin");//create a file on SD card that stores a unique file ID from 1 to 2^32 - 1 (in fact 1 to 99999)
  pre_allocate_lookup_tables(lookup_serial, lookup_TFT_RGB565, v_min, v_max);//pre allocate tables for TFT and serial output auto contrast
  // presets the exposure time before displaying to avoid unpleasing result
  for (int i = 1; i < 25; i++) {
    take_a_picture();
    auto_exposure(camReg, CamData, v_min, v_max);
  }
}

void loop()
{
  currentTime = millis();
  take_a_picture(); //data in memory for the moment, one frame
  auto_exposure(camReg, CamData, v_min, v_max); // Deals with autoexposure (registers 2 and 3) to target a mid voltage
  current_exposure = get_exposure(camReg);//get the current exposure register
  if (current_exposure > 0x0FFF) sprintf(exposure_string, "Exposure: %X", current_exposure); //concatenate string for display
  if (current_exposure <= 0x0FFF) sprintf(exposure_string, "Exposure: 0%X", current_exposure); //concatenate string for display;
  if (current_exposure <= 0x00FF) sprintf(exposure_string, "Exposure: 00%X", current_exposure); //concatenate string for display;
  if (current_exposure <= 0x000F) sprintf(exposure_string, "Exposure: 000%X", current_exposure); //concatenate string for display;
  //dump_data_to_serial(CamData);//dump data to serial for debugging - you can use the Matlab code ArduiCam_Matlab.m into the repo to probe the serial and plot images

  img.fillSprite(TFT_BLACK);// prepare the image in ram
  for (int16_t x = 1; x < 128 ; x++) {
    for (int16_t y = 0; y < 120; y++) {
      img.drawPixel(x, y + 16, lookup_TFT_RGB565[CamData[x + y * 128]]);
    }
  }

  if (recording == 0) {//just put informations to the ram too
    display_informations_idle();
  }
  if (recording == 1) {// prepare data for recording mode
    if ((currentTime - previousTime) > deadtime) {// Wait for deadtime set in config.txt
      Next_ID++;// update the file number
      previousTime = currentTime;
      sprintf(storage_file_name, "/%06d/%09d.bmp", Next_dir, Next_ID);//update filename

      if (HDR_mode == 0) {
        gpio_put(RED, 1);
        for (int i = 0; i < 128 * 128; i++) {
          BmpData[i] = lookup_serial[CamData[i]];//to get data with autocontrast
        }
      }

      if (HDR_mode == 1) {
        img.setTextColor(TFT_BLUE);
        img.setCursor(0, 16);
        img.println(F("HDR in acquisition"));
        img.pushSprite(0, 0);// dump image to display
        gpio_put(RED, 1);
        for (int i = 0; i < 128 * 128; i++) {//get the current picture with current exposure
          HDRData[i] = lookup_serial[CamData[i]];//while applying autocontrast
        }
        current_exposure = get_exposure(camReg);//get the current exposure register
        double exposure_list[7] = {0.5, 0.69, 0.79, 1, 1.26, 1.44, 2};
        for (int i = 0; i < 7; i++) {
          push_exposure(camReg, current_exposure, exposure_list[i]);//vary the exposure
          take_a_picture();
          for (int i = 0; i < 128 * 128; i++) {
            HDRData[i] = HDRData[i] + lookup_serial[CamData[i]]; //sum data while applying autocontrast
          }
        }
        //now time to average all this shit
        for (int i = 0; i < 128 * 128; i++) {
          BmpData[i] = HDRData[i] >> 3; //8 pictures so divide by 8
        }
        push_exposure(camReg, current_exposure, 1); //rewrite the old register
      }

      File dataFile = SD.open(storage_file_name, FILE_WRITE);
      // if the file is writable, write to it:
      if (dataFile) {
        dataFile.write(BMP_header, 1078);//fixed header for 128*120 image
        dataFile.write(BmpData, 128 * 120); //removing last tile line
        dataFile.close();
      }
      gpio_put(RED, 0);
    }
    //sprintf(storage_file_name, "/%06d/%09d.bmp", Next_dir, Next_ID);
    display_informations_recording();
  }

  img.pushSprite(0, 0);// dump image to display

  if ((gpio_get(PUSH) == 1) && recording == 0) { // we want to record: get file/directory#
    Next_ID = get_next_ID("/ID_storage.bin");//get the file number on SD card
    Next_dir = get_next_dir("/ID_storage.bin");//get the folder number on SD card
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
    store_next_ID("/ID_storage.bin", Next_ID, Next_dir);//store last known file/directory# to SD card
    gpio_put(RED, 0);
    recording = 0;
    delay(debouncing_delay);//get the folder number on SD card
  }

  if (recording == 0) { // Change HDR<->one frame modes
    if (gpio_get(HDR) == 1) {
      HDR_mode = !HDR_mode;
      delay(debouncing_delay);
    }
    if (gpio_get(BORDER) == 1) {
      BORDER_mode = !BORDER_mode;
      if (BORDER_mode==1) camReg[1]=0b11101000;
      if (BORDER_mode==0) camReg[1]=0b00001000;
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

void auto_exposure(unsigned char camReg[8], unsigned char CamData[128 * 128], unsigned char v_min, unsigned char v_max) {
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
  if ((error <= 80) && (error >= 20))    new_regs = exp_regs * 1.3;
  if ((error >= -80) && (error <= -20))  new_regs = exp_regs / 1.3;
  if ((error <= 20) && (error >= 10))     new_regs = exp_regs * 1.03;
  if ((error >= -20) && (error <= -10))   new_regs = exp_regs / 1.03;
  if ((error <= 10) && (error >= 1))   new_regs = exp_regs + 1;
  if ((error >= -10) && (error <= 1))  new_regs = exp_regs - 1;
  // The sensor is limited to 0xFFFF (about 1 second) in exposure but also has strong artifacts below 0x10 (256 µs).
  // Each step is 16 µs
  if (new_regs < 0x10) {//minimum of the sensor, below there are verticals artifacts
    new_regs = 0x10;
  }
  if (new_regs > 0xFFFF) {//maximum of the sensor, about 1 second
    new_regs = 0xFFFF;
  }
  camReg[2] = int(new_regs / 256);
  camReg[3] = int(new_regs - camReg[2] * 256);
}

void push_exposure(unsigned char camReg[8], unsigned int current_exposure, double factor) {
  double new_regs;
  new_regs = current_exposure * factor;
  if (new_regs < 0x10) {//minimum of the sensor, below there are verticals artifacts
    new_regs = 0x10;
  }
  if (new_regs > 0xFFFF) {//maximum of the sensor, about 1 second
    new_regs = 0xFFFF;
  }
  camReg[2] = int(new_regs / 256);
  camReg[3] = int(new_regs - camReg[2] * 256);
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

//////////////////////////////////////////////Output stuff///////////////////////////////////////////////////////////////////////////////////////////
void pre_allocate_lookup_tables(unsigned char lookup_serial[256], unsigned int lookup_TFT_RGB565[256], unsigned char v_min, unsigned char v_max) {
  unsigned char color, red, green, blue;
  unsigned int Rgb565;
  double gamma_pixel;
  for (int i = 0; i < 256; i++) {
    if (i < v_min) {
      lookup_serial[i] = 0x00;
      lookup_TFT_RGB565[i] = 0x0000;
    }
    if ((i >= v_min) && (i <= v_max)) {
      gamma_pixel = ((i - double(v_min)) / (double(v_max) - double(v_min))) * 255;
      lookup_serial[i] = int(gamma_pixel);
      color = int(gamma_pixel);
      red = color;     green = color;     blue = color;
      Rgb565 = (((red & 0b11111000) << 8) + ((green & 0b11111100) << 3) + (blue >> 3));
      lookup_TFT_RGB565[i] = Rgb565;
    }
    if (i > v_max) {
      lookup_serial[i] = 0xFF;
      lookup_TFT_RGB565[i] = 0xFFFF;
    }
  }
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
    Serial.println("Fresh configuration file created on SD card");
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
void display_informations_recording() {
  img.setTextColor(TFT_CYAN);
  img.setCursor(0, 0);
  img.println(F(exposure_string));
  img.setTextColor(TFT_RED);
  img.setCursor(0, 8);
  img.println(F("Recording Mode"));
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
}

void display_informations_idle() {
  img.setTextColor(TFT_CYAN);
  img.setCursor(0, 0);
  img.println(F(exposure_string));
  img.setTextColor(TFT_GREEN);
  img.setCursor(0, 8);
  img.println(F("Display Mode"));
  img.setTextColor(TFT_WHITE);
  img.setCursor(0, 136);
  img.println(F(storage_file_name));
  img.setTextColor(TFT_BLUE);
  img.setCursor(0, 144);
  img.println(F(storage_deadtime));
  img.setTextColor(TFT_PURPLE);
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
}

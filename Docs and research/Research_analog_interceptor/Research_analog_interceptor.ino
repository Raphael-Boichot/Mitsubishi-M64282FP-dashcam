//By Raphaël BOICHOT, made around 2023-04-27
//this version is to use with the Matlab code to test the effect of registers
//it outputs data over the serial and diplays raw data on screen

#include "hardware/adc.h" //the GPIO commands are here
#include "pico/stdlib.h"


// the order of pins has no importance except that VOUT must be on some ADC
const uint VOUT =  26; //to pi pico pin GPIO26/A0 Analog signal from sensor, read shortly after clock is set low, native 12 bits, converted to 8 bits

//the following pins must be shifted to 3.3V pico side<->5V sensor side
const uint READ =  7;   //to pi pico pin GPIO7 Read image signal, goes high on rising clock
const uint CLOCK = 8;   //to pi pico pin GPIO8 Clock input, pulled down internally, no specification given for frequency
const uint LED =   15; //to pi pico pin GPIO15 indicate exposure delay for the sensor <-> GND

const unsigned char LUT_serial[16] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46}; //HEX characters in ASCII table
unsigned char CamData[128 * 128];// sensor data in 8 bits per pixel
bool new_state = 0;
bool old_state = 0;

void setup()
{
  //digital stuff
  gpio_init(LED);       gpio_set_dir(LED, GPIO_OUT);
  gpio_init(READ);      gpio_set_dir(READ, GPIO_IN); gpio_pull_down(READ);
  gpio_init(CLOCK);     gpio_set_dir(CLOCK, GPIO_IN); gpio_pull_down(CLOCK);

  //analog stuff
  adc_gpio_init(VOUT);  adc_select_input(0);
  adc_init();
//set_sys_clock_khz(300000,true);
delay(2000);
  Serial.begin(2000000);
}

void loop()
{
  while (1) {
    new_state = gpio_get(READ);
    if ((new_state == 1) & (old_state == 0)) {
      Serial.println("Read detected");
      for (int i = 0; i < 128 * 128; i++) {
        CamData[i] = adc_read() >> 4;
      }
      Serial.println("End of Reading");
      dump_data_to_serial( CamData);
    }
    old_state = new_state;
  }
}

void dump_data_to_serial(unsigned char CamData[128 * 128]) {
  char pixel;
  for (int i = 0; i < 128 * 128; i++) {
    pixel = CamData[i]; //to get data without autocontrast
    if (pixel <= 0x0F) Serial.print('0');
    Serial.print(pixel, HEX);
    Serial.print(" ");
  }
  Serial.println("");
}

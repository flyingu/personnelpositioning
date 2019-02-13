#include "led.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include <stdbool.h>
#include "main.h"
void blueled_open(void){
  nrf_gpio_cfg_output(BLUE_LED);
  nrf_gpio_pin_set(BLUE_LED);
}
void blueled_init(void){
  nrf_gpio_cfg_output(BLUE_LED);
  nrf_gpio_pin_clear(BLUE_LED);
}
void blueled_close(void){
  nrf_gpio_cfg_output(BLUE_LED);
  nrf_gpio_pin_clear(BLUE_LED);
}
bool blueled_read(void){
  if(nrf_gpio_pin_read(BLUE_LED)){
    return true;
  }
  else
    return false;
}
void blueled_work(void){
  nrf_gpio_cfg_output(BLUE_LED);
  nrf_gpio_pin_set(BLUE_LED);
  nrf_delay_ms(200);
  nrf_gpio_pin_clear(BLUE_LED);
  nrf_delay_ms(200);
  nrf_gpio_pin_set(BLUE_LED);
  nrf_delay_ms(200);
  nrf_gpio_pin_clear(BLUE_LED);
  nrf_delay_ms(200);
  nrf_gpio_pin_set(BLUE_LED);
  nrf_delay_ms(200);
  nrf_gpio_pin_clear(BLUE_LED);
}
//
void redled_open(void){
  nrf_gpio_cfg_output(RED_LED);
  nrf_gpio_pin_set(RED_LED);
}
void redled_init(void){
  nrf_gpio_cfg_output(RED_LED);
  nrf_gpio_pin_clear(RED_LED);
}
void redled_close(void){
  nrf_gpio_cfg_output(RED_LED);
  nrf_gpio_pin_clear(RED_LED);
}
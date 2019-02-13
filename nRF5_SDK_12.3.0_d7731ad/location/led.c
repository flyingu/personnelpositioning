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

void greenled_open(void){
  nrf_gpio_cfg_output(GREEN_LED);
  nrf_gpio_pin_set(GREEN_LED);
}
void greenled_init(void){
  nrf_gpio_cfg_output(GREEN_LED);
  nrf_gpio_pin_clear(GREEN_LED);
}
void greenled_close(void){
  nrf_gpio_cfg_output(GREEN_LED);
  nrf_gpio_pin_clear(GREEN_LED);
}
bool greenled_read(void){
  if(nrf_gpio_pin_read(GREEN_LED)){
    return true;
  }
  else
    return false;
}
void greenled_work(uint8_t time){
  nrf_gpio_cfg_output(GREEN_LED);
  for(uint8_t i=time;i>0;i--){
    nrf_gpio_pin_set(GREEN_LED);
    nrf_delay_ms(400);
    nrf_gpio_pin_clear(GREEN_LED);
    nrf_delay_ms(200);
  }
}
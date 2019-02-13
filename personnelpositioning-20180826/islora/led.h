#ifndef LED_H
#define LED_H
#include <stdbool.h>
#include <stdint.h>
  void blueled_open(void);
  void blueled_init(void);
  void blueled_close(void);
  bool blueled_read(void);
 void blueled_work(uint8_t time);

  void redled_open(void);
  void redled_init(void);
  void redled_close(void);
  void redled_work(uint8_t time);
  
  void greenled_open(void);
  void greenled_init(void);
  void greenled_close(void);
  bool greenled_read(void);
  void greenled_work(uint8_t time);
#endif
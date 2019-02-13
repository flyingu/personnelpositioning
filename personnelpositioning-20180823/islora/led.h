#ifndef LED_H
#define LED_H
#include <stdbool.h>
  void blueled_open(void);
  void blueled_init(void);
  void blueled_close(void);
  bool blueled_read(void);
  void blueled_work(void);

  void redled_open(void);
  void redled_init(void);
  void redled_close(void);
#endif
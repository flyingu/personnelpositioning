#ifndef _GPS_H
#define _GPS_H
#include "stdint.h"
extern uint8_t gps_str[256];
extern uint8_t gps_str_ptr;
void GpsInit(void);
void OpenGps(void);
void CloseGps(void);
void ParseGps(void);
#endif

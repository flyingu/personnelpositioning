#ifndef __GPSPROTOCOL__H__
#define __GPSPROTOCOL__H__
#include "stdint.h"
typedef enum 
{
   PROTOCOL_TYPE_SOS	        = (unsigned char)0x20,
   PROTOCOL_TYPE_GPS	        = (unsigned char)0x19,
   PROTOCOL_TYPE_GPSTIMECFG	= (unsigned char)0x80,
   PROTOCOL_TYPE_LORACFG	= (unsigned char)0x91

   
}MY_PROTOCOL_TYPE;
typedef union{
  uint8_t arr[4];
  uint32_t i32;
}myint32;
extern uint8_t proto_data_len;
extern uint8_t proto_data_str[255];

uint8_t Getcheck(uint8_t* re,uint8_t len);
void MakeUpData(uint8_t* data,uint8_t len);
void MakeUpCommand(uint8_t* data,uint8_t len,MY_PROTOCOL_TYPE protocoltype);
uint8_t ParseData(uint8_t* da2,uint8_t* did,uint16_t len);
#endif


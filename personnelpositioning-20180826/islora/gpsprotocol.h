#ifndef __GPSPROTOCOL__H__
#define __GPSPROTOCOL__H__
#include "stdint.h"
typedef enum 
{
   PROTOCOL_TYPE_POWER_LOW      = (unsigned char)0x0a,
   PROTOCOL_TYPE_SOS	        = (unsigned char)0x0f,
   PROTOCOL_TYPE_GPS	        = (unsigned char)0x19,
   PROTOCOL_TYPE_CFGGPSTIME	= (unsigned char)0x90,
   PROTOCOL_TYPE_GPS_RESPONSE   = (unsigned char)0x9e,
   PROTOCOL_TYPE_CFG_RESPONSE   = (unsigned char)0x9f,
   
}MY_PROTOCOL_TYPE;
typedef union{  
  uint8_t arr[4];
  uint32_t i32;
}myint32;
typedef union{  
  uint8_t arr[2];
  uint16_t i16;
}myint16;
extern uint8_t proto_data_len;
extern uint8_t proto_data_str[255];

uint8_t Getcheck(uint8_t* re,uint8_t len);
void MakeUpData(uint8_t* data,uint8_t len);
void MakeUpCommand(uint8_t* data,uint8_t len,MY_PROTOCOL_TYPE protocoltype);
uint8_t ParseData(uint8_t* da2,uint8_t* did,uint16_t len);
#endif


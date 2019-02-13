#ifndef __MYFLASH__H__
#define __MYFLASH__H__
#define FLASHDATA_SIZE 6
#include "stdint.h"
#include "gpsprotocol.h"
typedef union {
  struct{
    uint8_t flashdataempty;
    myint32 gpslooptime;
    myint32 worktime;
    myint32 resttime;//10
    uint8_t fre[3];
    uint8_t sf;
    uint8_t cr;
    uint8_t bw;
    uint8_t power;//17
  }flash_store_data_s;
  uint32_t flashdata[FLASHDATA_SIZE];
}flash_store_data_t;

void myflash_init(void);
void myflash_write(uint32_t  * data, uint16_t data_len);
void myflash_read(uint32_t *dest_addr, uint16_t data_len);
void flashdata_load(void);
#endif
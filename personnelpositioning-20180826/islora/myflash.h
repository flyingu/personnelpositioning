#ifndef __MYFLASH__H__
#define __MYFLASH__H__
#define FLASHDATA_SIZE 5
#include "stdint.h"
#include "gpsprotocol.h"
typedef union {
  struct{
    uint8_t flashdataempty;//1
    myint32 gpslooptime;//4
    myint32 worktime;//4
    myint32 resttime;//4
    uint8_t fre[3];//3
    uint8_t sf;//1
    uint8_t cr;//1
    uint8_t bw;//1
    uint8_t power;//1
  }flash_store_data_s;
  uint32_t flashdata[FLASHDATA_SIZE];
}flash_store_data_t;
void fstorage_test(void);
void myflash_init(void);
void myflash_write(uint32_t  *data, uint16_t data_len);
void myflash_read(uint32_t *dest_addr, uint16_t data_len);
void flashdata_load(void);
#endif
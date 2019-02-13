#include "myflash.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_timer.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "fstorage.h"
#include "section_vars.h"
#include "main.h"

#define PAGE_SIZE_WORDS 256
#define NUM_PAGES 4
 uint8_t fs_callback_flag;
flash_store_data_t flash_store_data;
flash_store_data_t flash_store_data_default={ 
  .flash_store_data_s.flashdataempty=0xa2,//1
  .flash_store_data_s.gpslooptime.i32=0x00000060, //4
  .flash_store_data_s.worktime.i32=30600,//4
  .flash_store_data_s.resttime.i32=63000, //4
  .flash_store_data_s.fre={0x75,0xe0,0x00},//3
  .flash_store_data_s.sf=12,//1
  .flash_store_data_s.cr=2, //1
  .flash_store_data_s.bw=2,//1
  .flash_store_data_s.power=7,//1
};
//uint32_t *myflash_start_addr= (uint32_t *)0x3fc00;//将flash地址付给指针
static void fs_evt_handler(fs_evt_t const * const evt, fs_ret_t result)
{
  if (result != FS_SUCCESS){

  }
  else{
   fs_callback_flag = 0;
  }
}
FS_REGISTER_CFG(fs_config_t fs_config) =
  {
    .callback  = fs_evt_handler, // Function for event callbacks.
    .num_pages = NUM_PAGES,      // Number of physical flash pages required.
    .priority  = 0xFE            // Priority for flash usage.
  };
//static uint32_t const * address_of_page(uint16_t page_num)
//{
//    return fs_config.p_start_addr + (page_num * PAGE_SIZE_WORDS);
//}

void myflash_erase(void){
  fs_ret_t ret;
    //printf("Erasing a flash page at address 0x%X\r\n", (uint32_t)fs_config.p_start_addr);
    fs_callback_flag = 1;
    ret = fs_erase(&fs_config, fs_config.p_start_addr, 1,NULL);
    if (ret != FS_SUCCESS)
    {
      //bsp_indication_set(BSP_INDICATE_FATAL_ERROR);
    }
    while(fs_callback_flag == 1)  { 
      power_manage(); }
   
}
void myflash_init(void){
  
  fs_ret_t ret = fs_init();
  APP_ERROR_CHECK(ret);
  if (ret != FS_SUCCESS)
  {
      // An error occurred.
  }
}
//
/** @brief Function for filling a page in flash with a value.
 *
 * @param[in] address Address of the first word in the page to be filled.
 * @param[in] value Value to be written to flash.
 */
void myflash_write(uint32_t  * data, uint16_t data_len)
{
  fs_ret_t ret;
  myflash_erase();
  //printf("Writing data 0x%X to address 0x%X\r\n", data, (uint32_t)fs_config.p_start_addr);
  fs_callback_flag = 1;
  ret = fs_store(&fs_config, fs_config.p_start_addr, data, data_len,NULL);      //Write data to memory address 0x0003F000. Check it with command: nrfjprog --memrd 0x0003F000 --n 16
  if (ret != FS_SUCCESS)
  {
    //bsp_indication_set(BSP_INDICATE_FATAL_ERROR);
  }
  while(fs_callback_flag == 1)  { power_manage(); }
}

void myflash_read(uint32_t *dest_addr, uint16_t data_len)
{
   //Read the first 4 words of the page
    //printf("Data read from flash address 0x%X: ", (uint32_t)fs_config.p_start_addr);
    for(int i=0; i<data_len; i++)
    {
       dest_addr[i] = *(fs_config.p_start_addr + i);
      //printf("%X ", flash_data[i]);
    }
    //printf("\r\n");

}
void flashdata_load(void){
  //myflash_read(flash_store_data.flashdata, FLASHDATA_SIZE);
 // if(flash_store_data.flash_store_data_s.flashdataempty!=0xaa){
    myflash_write(flash_store_data_default.flashdata, FLASHDATA_SIZE);
    myflash_read(flash_store_data.flashdata, FLASHDATA_SIZE);
  //}
}


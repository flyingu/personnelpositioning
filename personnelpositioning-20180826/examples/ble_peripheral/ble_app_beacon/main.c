/**
* Copyright (c) 2014 - 2017, Nordic Semiconductor ASA
* 
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
* 
* 2. Redistributions in binary form, except as embedded into a Nordic
*    Semiconductor ASA integrated circuit in a product or a software update for
*    such product, must reproduce the above copyright notice, this list of
*    conditions and the following disclaimer in the documentation and/or other
*    materials provided with the distribution.
* 
* 3. Neither the name of Nordic Semiconductor ASA nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
* 
* 4. This software, with or without modification, must only be used with a
*    Nordic Semiconductor ASA integrated circuit.
* 
* 5. Any software provided in binary form under this license must not be reverse
*    engineered, decompiled, modified and/or disassembled.
* 
* THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

/** @file
*
* @defgroup ble_sdk_app_beacon_main main.c
* @{
* @ingroup ble_sdk_app_beacon
* @brief Beacon Transmitter Sample Application main file.
*
* This file contains the source code for an Beacon transmitter sample application.
*/
#include "main.h"
#include <stdbool.h>
#include <stdint.h>
#include "ble_advdata.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "bsp.h"
#include "app_timer.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "sx1278.h"
#include "nrf_delay.h"
#include "app_uart.h"
#include "gpsprotocol.h"
#include "app_fifo.h"
#include "nrf_drv_wdt.h"
#include "nrf_drv_gpiote.h"
#include "app_gpiote.h"
#include "app_button.h"
#include "nrf_drv_adc.h"
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#include "led.h"
#include "nrf_log_ctrl.h"
#include "myflash.h"
#include "fstorage.h"
#define CENTRAL_LINK_COUNT              0                                 /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           0                                 /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                 /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define APP_CFG_NON_CONN_ADV_TIMEOUT    0                                 /**< Time for which the device must be advertising in non-connectable mode (in seconds). 0 disables timeout. */
#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(500, UNIT_0_625_MS) /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */

#define DEAD_BEEF                       0xDEADBEEF                        /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define APP_TIMER_PRESCALER             0                                 /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                 /**< Size of timer operation queues. */

#define DEVICE_NAME                     "UBIPP-10007"                               /**< Name of device. Will be included in the advertising data. */

static ble_gap_adv_params_t m_adv_params;                                 /**< Parameters to be passed to the stack when starting advertising. */
//**************GPS**************************************************
#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */
uint8_t gps_timeout=0;
uint8_t gps_finish_flag=0;
uint8_t gps_str_ptr=0;
uint8_t gps_str[256]={0};
void gps_close(void);
bool gps_status=false;
bool lora_status=false;
unsigned char battery_vol_value[1]={0x1b};
uint8_t MyGpsId[8]={0x83,0x60,0x18,0x10,0x03,0x12,0x00,0x07};
APP_TIMER_DEF(m_location_timer_id);
APP_TIMER_DEF(m_rtc_timer_id);
APP_TIMER_DEF(m_adc_timer_id);
APP_TIMER_DEF(m_lora_recv_timer_id);
nrf_drv_wdt_channel_id m_channel_id;  
uint8_t cmd_number=0;
uint8_t mode=0;
uint8_t seg_count=0;		//逗号计数加1
uint8_t byte_count=0;
unsigned char buf_full;		//1：整句接收完成，相应数据有效。0：缓存数据无效。
uint8_t gpscount=0;
//uint32_t gpstimeout=8550;//25250;//29250;//40秒//8550;//10秒58500;//60秒//8550;//10秒
uint8_t timeout=0;
uint32_t gpslooptime=20;
uint8_t sos_flag=0;
uint8_t response_message[5]={0};
uint8_t sos_message[4]={0x01,0x01,0x01,0x01};
uint8_t button_count=0;
#define APP_GPIOTE_MAX_USERS                 1  //Maximum number of users of the GPIOTE handler.
uint8_t sendmessage[16]={"Failed to locate"};
uint8_t TimeSeconds=0;
uint8_t TimeMinute=59;
uint8_t TimeHour=8;
#define TIME_LEVEL_MEAS_INTERVAL APP_TIMER_TICKS(1000,APP_TIMER_PRESCALER)
#define TIME_LEVEL_RTC_INTERVAL APP_TIMER_TICKS(1000,APP_TIMER_PRESCALER)
#define TIME_LEVEL_ADC_INTERVAL APP_TIMER_TICKS(60000,APP_TIMER_PRESCALER)
#define TIME_LEVEL_LORARECV_INTERVAL APP_TIMER_TICKS(9000,APP_TIMER_PRESCALER)
bool worktime=false;
uint8_t firsttime=0;
uint8_t lorarecv_timeout=0;
locationcard_evt_type_t locationcard_event; 
uint8_t sx1278_tx_count=0;
uint8_t sx1278_tx_vol_count=0;
extern flash_store_data_t flash_store_data;
uint8_t recv_sn[4]={0}; 
/** ----------------------------------------------adc--------------------------------------------------------*/

#define ADC_BUFFER_SIZE 1                               /**< Size of buffer for ADC samples.  */
nrf_adc_value_t       adc_buffer[ADC_BUFFER_SIZE]; /**< ADC buffer. */
static nrf_drv_adc_channel_t m_channel_config = NRF_DRV_ADC_DEFAULT_CHANNEL(NRF_ADC_CONFIG_INPUT_2); /**< Channel instance. Default configuration used. */
uint8_t adc_buff[10]={0};
uint8_t adc_index=0;
#define ADC_PRE_SCALING_COMPENSATION    4                                            /**< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.*/
#define ADC_REF_VOLTAGE_IN_MILLIVOLTS   1200                                         /**< Reference voltage (in milli volts) used by ADC while doing conversion. */
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) ((((ADC_VALUE) * ADC_REF_VOLTAGE_IN_MILLIVOLTS) / 1023) * ADC_PRE_SCALING_COMPENSATION)
nrf_adc_value_t       adc_value;
myint16 batteryvalue;
bool AdcTimerStart_Flag=false;
void lora_close(void){
  nrf_gpio_cfg_output(CTL_LR_VDD);
  nrf_gpio_pin_set(CTL_LR_VDD);
  lora_status=false;
}
void lora_open(void){
  nrf_gpio_cfg_output(CTL_LR_VDD);
  nrf_gpio_pin_clear(CTL_LR_VDD);
  lora_status=true;
}

void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
  app_error_handler(DEAD_BEEF, line_num, p_file_name);
}
 void gap_params_init(void)
{
  uint32_t err_code;
  ble_gap_conn_sec_mode_t     sec_mode;
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
  err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)DEVICE_NAME,
                                        strlen(DEVICE_NAME));
  APP_ERROR_CHECK(err_code);
}
/**@brief Function for initializing the Advertising functionality.
*
* @details Encodes the required advertising data and passes it to the stack.
*          Also builds a structure to be passed to the stack when starting advertising.
*/
 void advertising_init(void)
{
  uint32_t      err_code;
  ble_advdata_t advdata;
  uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
  int8_t tx_power_level = 0;
  // Build and set advertising data.
  memset(&advdata, 0, sizeof(advdata));
  advdata.p_tx_power_level	= &tx_power_level;
  advdata.name_type             = BLE_ADVDATA_FULL_NAME;
  advdata.flags                 = flags;
  sd_ble_gap_tx_power_set(tx_power_level);
  err_code = ble_advdata_set(&advdata, NULL);
  APP_ERROR_CHECK(err_code);
  
  // Initialize advertising parameters (used when starting advertising).
  memset(&m_adv_params, 0, sizeof(m_adv_params));
  
  m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_NONCONN_IND;
  m_adv_params.p_peer_addr = NULL;                             // Undirected advertisement.
  m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
  m_adv_params.interval    = NON_CONNECTABLE_ADV_INTERVAL;
  m_adv_params.timeout     = APP_CFG_NON_CONN_ADV_TIMEOUT;
}


/**@brief Function for starting advertising.
*/
void advertising_start(void)
{
  uint32_t err_code;
  
  err_code = sd_ble_gap_adv_start(&m_adv_params);
  APP_ERROR_CHECK(err_code);
  
  err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
  APP_ERROR_CHECK(err_code);
}
static void sys_evt_dispatch(uint32_t sys_evt)
{
  //  ble_advertising_on_sys_evt(sys_evt);
    fs_sys_event_handler(sys_evt);
}

/**@brief Function for initializing the BLE stack.
*
* @details Initializes the SoftDevice and the BLE event interrupt.
*/
 void ble_stack_init(void)
{
  uint32_t err_code;
  
  nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
  
  // Initialize the SoftDevice handler module.
  SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
  
  ble_enable_params_t ble_enable_params;
  err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                  PERIPHERAL_LINK_COUNT,
                                                  &ble_enable_params);
  APP_ERROR_CHECK(err_code);
  
  //Check the ram settings against the used number of links
  CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);
  
  // Enable BLE stack.
  err_code = softdevice_enable(&ble_enable_params);
  APP_ERROR_CHECK(err_code);
  err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}

/**@brief   Function for handling app_uart events.
*
* @details This function will receive a single character from the app_uart module and append it to
*          a string. The string will be be sent over BLE when the last character received was a
*          'new line' i.e '\r\n' (hex 0x0D) or if the string has reached a length of
*          @ref NUS_MAX_DATA_LENGTH.
*/
//uint8_t gps_a_count=0;
//uint8_t rmc_str[100];
//uint8_t rmc_ptr=0;
//char gsa_str[100];
//uint8_t gsa_ptr=0;
//
//void uart_event_handle(app_uart_evt_t * p_event)
//{  
//  if(p_event->evt_type==APP_UART_DATA_READY){
//     nrf_drv_wdt_channel_feed(m_channel_id);  
//   
//    UNUSED_VARIABLE(app_uart_get(&gps_str[gps_str_ptr]));
//    switch(gps_str[gps_str_ptr]){
//    case '$':
//      cmd_number=0;
//      //命令类型清空
//      mode=1;			//接收命令模式
//      gps_str[0]='$';
//      gps_str_ptr=1;		                      
//      break;
//    case '*':
//      if(cmd_number==3){   //GNRMC
//        NRF_LOG_INFO("cmd_number==3\r\n");
//        if((gps_str[gps_str_ptr-1]=='A')||(gps_str[gps_str_ptr-1]=='D')){ 
//          memcpy(rmc_str,gps_str,gps_str_ptr);
//          rmc_ptr=gps_str_ptr;
//          memset(gps_str,0,sizeof(gps_str));
//          gps_str_ptr=0;
//        }
//      }
//      if(cmd_number==1){   //GNGSA
//          memcpy(gsa_str,gps_str,gps_str_ptr);
//          gsa_ptr=gps_str_ptr;
//          char result[4] = {""};
//          strncpy(result, gsa_str+(gps_str_ptr-14), 4);
//          double d_pdop=atof(result);
//          gps_str_ptr=0;       
//          if(rmc_ptr!=0&&d_pdop<3.0){
//            rmc_str[rmc_ptr++]=',';
//            for(uint8_t i=0;i<4;i++){
//              rmc_str[rmc_ptr++]=result[i];
//            } 
//            gps_close();
//            locationcard_event=GPS_REV_DONE_EVT; 
//          }
//          
//      }
//      mode=0;
//      cmd_number=0;
//      
//      break;
//    default:
//      if(mode==1)	//命令种类判断
//      {
//        //接收字符放入类型缓存
//        if(gps_str_ptr>=5)
//        {				//如果类型数据接收完毕，判断类型
//          if(gps_str[1]=='G'&&gps_str[2]=='N'&&gps_str[3]=='R'&&gps_str[4]=='M'&&gps_str[5]=='C')
//          {                 
//            cmd_number=3;//进入GNRMC 
//          }
//          				//如果类型数据接收完毕，判断类型
//          if(gps_str[1]=='G'&&gps_str[2]=='N'&&gps_str[3]=='G'&&gps_str[4]=='S'&&gps_str[5]=='A')
//          {                 
//            cmd_number=1;//进入GNRMC 
//          }
//        }              
//      } 
//      
//      gps_str_ptr++;		//接收数位加1
//      break;
//    }
//    
//  }
//}
/**@snippet [Handling the data received over UART] */
uint8_t gps_a_count=0;
void uart_event_handle(app_uart_evt_t * p_event)
{  
  if(p_event->evt_type==APP_UART_DATA_READY){
     nrf_drv_wdt_channel_feed(m_channel_id);  
   
    UNUSED_VARIABLE(app_uart_get(&gps_str[gps_str_ptr]));
    switch(gps_str[gps_str_ptr]){
    case '$':
      cmd_number=0;		//命令类型清空
      mode=1;			//接收命令模式
      gps_str[0]='$';
      gps_str_ptr=1;		                      
      break;
    case '*':
      if(cmd_number==3){   //GNRMC
      // NRF_LOG_INFO("cmd_number==3\r\n");
        if((gps_str[gps_str_ptr-1]=='A')||(gps_str[gps_str_ptr-1]=='D')){
          //NRF_LOG_INFO("location successful\r\n");
          gps_a_count++;
          if(gps_a_count==30){
            gps_a_count=0;
            gps_close();
            gpscount=0;
            locationcard_event=GPS_REV_DONE_EVT;  
          }    
        }
        else if(gpscount==120){
          gps_close();
          gpscount=0;
          gps_a_count=0;             
          locationcard_event=GPS_REV_DONE_EVT;
        }else{
          gpscount++;
          gps_a_count=0;
        }
      }
      mode=0;
      break;
    default:
      if(mode==1)	//命令种类判断
      {
        //接收字符放入类型缓存
        if(gps_str_ptr>=5)
        {				//如果类型数据接收完毕，判断类型
          if(gps_str[1]=='G'&&gps_str[2]=='N'&&gps_str[3]=='R'&&gps_str[4]=='M'&&gps_str[5]=='C')
          {                 
            cmd_number=3;//进入GNRMC 
          }
        }              
      }            
      gps_str_ptr++;		//接收数位加1
      break;
    }
    
  }
}

/**@snippet [Handling the data received over UART] */
/**@brief  Function for initializing the UART module.
*/
/**@snippet [UART Initialization] */
void uart_init(void)
{
  uint32_t                     err_code;
  const app_uart_comm_params_t comm_params =
  {
    RX_PIN_NUMBER,
    NULL,
    NULL,
    NULL,
    APP_UART_FLOW_CONTROL_DISABLED,
    false,
    UART_BAUDRATE_BAUDRATE_Baud9600
  };
  
  APP_UART_FIFO_INIT( &comm_params,
                     UART_RX_BUF_SIZE,
                     UART_TX_BUF_SIZE,
                     uart_event_handle,
                     APP_IRQ_PRIORITY_LOWEST,
                     err_code);
  APP_ERROR_CHECK(err_code);
}

/**
*GPS
**/
void gps_init(void){
  //NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);
  nrf_gpio_cfg_output(CTL_GPS_VDD);
  nrf_gpio_pin_set(CTL_GPS_VDD);
}
void gps_close(void){
  NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);
  nrf_gpio_cfg_output(CTL_GPS_VDD);
  nrf_gpio_pin_set(CTL_GPS_VDD);
  gps_status=false;
 // app_uart_close();
}
void gps_open(void){
  //uart_init();
  NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
  nrf_gpio_cfg_output(CTL_GPS_VDD);
  nrf_gpio_pin_clear(CTL_GPS_VDD);
  gps_status=true;
  memset(gps_str,0,sizeof(gps_str));
}

/**@snippet [UART Initialization] */
/**@brief Function for doing power management.
*/
void power_manage(void)
{
  uint32_t err_code = sd_app_evt_wait();//sd_power_system_off();//
  APP_ERROR_CHECK(err_code);
}

/********************************************************************************/
//timer module/
void location_timers_stop(void)
{
  uint32_t err_code;
  err_code = app_timer_stop(m_location_timer_id);
  APP_ERROR_CHECK(err_code);
}
static void location_timeout_handler(void * p_context)
{
  UNUSED_PARAMETER(p_context);
  gpslooptime--;
  nrf_drv_wdt_channel_feed(m_channel_id);  
  if(gpslooptime==0){
    location_timers_stop(); 
    gps_open();  
  }
//  if(gpslooptime%30==0){
//    if(battery_vol_value[0]==0x1b){
//      blueled_work(5);
//    }
//  }
}
void location_timers_start(void)
{
  uint32_t err_code;
              gpslooptime= \
              ((uint32_t)flash_store_data.flash_store_data_s.gpslooptime.arr[2])*3600+ \
              ((uint32_t)flash_store_data.flash_store_data_s.gpslooptime.arr[1])*60+ \
              ((uint32_t)flash_store_data.flash_store_data_s.gpslooptime.arr[0]);
              if(gpslooptime<46){
                gpslooptime=1;
              }else{
                gpslooptime=gpslooptime-45;
              }
  err_code = app_timer_start(m_location_timer_id, TIME_LEVEL_MEAS_INTERVAL, NULL);
  
  APP_ERROR_CHECK(err_code);
}
void rtc_timers_stop(void)
{
  uint32_t err_code;
  err_code = app_timer_stop(m_rtc_timer_id);
  APP_ERROR_CHECK(err_code);
}
void rtc_timer_start(void){
  uint32_t err_code;
  err_code = app_timer_start(m_rtc_timer_id, TIME_LEVEL_RTC_INTERVAL, NULL);
  APP_ERROR_CHECK(err_code);
  
}
void update_rtc(void){
  TimeSeconds++;
  nrf_drv_wdt_channel_feed(m_channel_id);  //一分钟喂狗一次
  if(TimeSeconds==60){
    TimeSeconds=0;
    TimeMinute++;
    if(TimeMinute==60){
      TimeMinute=0;
      TimeHour++;
      if(TimeHour==24){
        TimeHour=0;       
      }     
    }
  }
}
static void rtc_timeout_handler(void * p_context)
{
  UNUSED_PARAMETER(p_context);
  update_rtc();        
}

//***************************adc_timer******************************************
void adc_timers_stop(void)
{
  uint32_t err_code;
  err_code = app_timer_stop(m_adc_timer_id);
  APP_ERROR_CHECK(err_code);
}
void adc_timer_start(void){
  uint32_t err_code;
  
  err_code = app_timer_start(m_adc_timer_id, TIME_LEVEL_ADC_INTERVAL, NULL);
  APP_ERROR_CHECK(err_code);  
}
void adc_stop(void){
  nrf_drv_adc_uninit();
}
uint8_t battery_status=0x00;
/**
* @brief ADC interrupt handler.
*/
static void adc_event_handler(nrf_drv_adc_evt_t const * p_event)
{
  if (p_event->type == NRF_DRV_ADC_EVT_DONE)
  { 
    if(gps_status==false&&lora_status==false){
      if(p_event->data.done.p_buffer[0]<680){
        //greenled_work(5);
        battery_vol_value[0]=0x1b;
      }else{
       // greenled_close();
        battery_vol_value[0]=0x20;
      }   
      //batteryvalue.i16=ADC_RESULT_IN_MILLI_VOLTS(p_event->data.done.p_buffer[0]);
    //  NRF_LOG_HEXDUMP_INFO(batteryvalue.arr,2);
      locationcard_event=BATTERY_VOL_GET_EVT;
    }
    adc_stop();
  }
  
}
/**
* @brief ADC initialization.
*/
void adc_config(void)
{
  ret_code_t ret_code;
  nrf_drv_adc_config_t config = NRF_DRV_ADC_DEFAULT_CONFIG;
  
  ret_code = nrf_drv_adc_init(&config, adc_event_handler);
  APP_ERROR_CHECK(ret_code);
  nrf_drv_adc_channel_enable(&m_channel_config);
}
void adc_start(void){
   adc_config();
  APP_ERROR_CHECK(nrf_drv_adc_buffer_convert(&adc_value,ADC_BUFFER_SIZE));
  nrf_drv_adc_sample();                       
}


static void adc_timeout_handler(void * p_context)
{
  UNUSED_PARAMETER(p_context);
  adc_start(); 
}
//***************adc_timer****end***********************************************

//***************lora_recv_timer************************************************
static void lora_recv_timeout_handler(void * p_context){
  UNUSED_PARAMETER(p_context);
  lorarecv_timeout=1;
}
void lora_recv_timers_stop(void)
{
  uint32_t err_code;
  err_code = app_timer_stop(m_lora_recv_timer_id);
  lorarecv_timeout=0;
  
  APP_ERROR_CHECK(err_code);
}
void lora_recv_timer_start(void){
  uint32_t err_code;
  lorarecv_timeout=0;
  err_code = app_timer_start(m_lora_recv_timer_id, TIME_LEVEL_LORARECV_INTERVAL, NULL);
  APP_ERROR_CHECK(err_code);  
}
//***************lora_recv_timer****end*****************************************

static void timers_init(void)
{
  uint32_t err_code;
  APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
  err_code = app_timer_create(&m_location_timer_id,
                              APP_TIMER_MODE_REPEATED,
                              location_timeout_handler);
  err_code = app_timer_create(&m_rtc_timer_id,
                              APP_TIMER_MODE_REPEATED,
                              rtc_timeout_handler);
  err_code = app_timer_create(&m_adc_timer_id,
                              APP_TIMER_MODE_REPEATED,
                              adc_timeout_handler);
  err_code = app_timer_create(&m_lora_recv_timer_id,
                              APP_TIMER_MODE_SINGLE_SHOT,
                              lora_recv_timeout_handler);
  APP_ERROR_CHECK(err_code);
  
}

/********************************************************************************/


/*
* Handler to be called when button is pushed.
* param[in]   pin_no             The pin number where the event is genereated
* param[in]   button_action     Is the button pushed or released
*/
void button_handler(uint8_t pin_no, uint8_t button_action)
{
  if(button_action == APP_BUTTON_PUSH)
  {
    switch(pin_no)
    {
    case DIO0:
      SX1278_Interrupt();
      break;
    case button_1:
      sos_flag=1;
      locationcard_event=SOS_OCCURS_EVT;
      button_count++;
      break;     
    default:
      break;
    }
  }
}  
void button_init(void){
  uint32_t err_code;
  static app_button_cfg_t p_button[] = { 
    {button_1, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_handler},
    {DIO0, APP_BUTTON_ACTIVE_HIGH, NRF_GPIO_PIN_PULLUP, button_handler}
    
  };
  APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);  
  err_code = app_button_init(p_button, sizeof(p_button) / sizeof(p_button[0]), 50);
  APP_ERROR_CHECK(err_code);
  
  // Enabling the buttons.                                        
  err_code = app_button_enable();
  APP_ERROR_CHECK(err_code);
}

void wdt_event_handler(void){
  blueled_work(2);
  //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
}
void wdt_init(void){
  //配置WDT.
  uint32_t err_code;
  nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;//采用默认设置
  err_code = nrf_drv_wdt_init(&config, wdt_event_handler);//使用默认参数配置看门狗。即CPU睡眠时，看门狗保持运行；看门狗超时周期2秒
  APP_ERROR_CHECK(err_code);
  err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);//分配一个通道id
  APP_ERROR_CHECK(err_code);
  nrf_drv_wdt_enable();//使能看门狗
  
}



uint8_t stu_bq25070_read(void){
  uint8_t value=1;
  nrf_gpio_cfg_input(STU_BQ25070,NRF_GPIO_PIN_PULLUP);
  if(nrf_gpio_pin_read(STU_BQ25070)==0){
    value=0;
  }
  return value;
}

/**
* @brief Function for application main entry.
*/
uint8_t power=0;

int main(void)
{    
   uint32_t err_code;
  err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);
  timers_init();
  ble_stack_init();
  gap_params_init();
  advertising_init();
  myflash_init();
  flashdata_load();
  advertising_start();
  wdt_init();
  uart_init();
  button_init();
  locationcard_event=LOCATIONCARD_GPS_START_EVT;
  NRF_LOG_INFO("BLE Beacon started\r\n");
 
 // adc_start(); 
  for(;;){
    switch(locationcard_event){
    case BATTERY_VOL_GET_EVT:
      NRF_LOG_INFO("into BATTERY_VOL_GET_EVT\r\n")
      lora_close();
      gps_close();
      adc_start(); 
      nrf_delay_ms(2000); 
      if( battery_status!=battery_vol_value[0]){
        if(battery_vol_value[0]==0x1b){
          flash_store_data.flash_store_data_s.gpslooptime.i32=0x00000A00;//10分钟
        }else if(battery_vol_value[0]==0x20){
          flashdata_load();
        }
        battery_status=battery_vol_value[0];
        locationcard_event=BATTERY_VOL_LOW;
      }else{
        locationcard_event=LOCATIONCARD_GPS_START_EVT;
      }
      break;
    case LORA_REV_READY_EVT:
      NRF_LOG_INFO("into LORA_REV_READY_EVT\r\n");
      gps_close();
      lora_recv_timer_start();
      nrf_drv_wdt_channel_feed(m_channel_id);  
      while((recv_length==0)&&(lorarecv_timeout==0));
        NRF_LOG_INFO("into while\r\n");
        lorarecv_timeout=0;
        lora_recv_timers_stop();   
        if(recv_length>0){
         
           uint8_t commandtype=ParseData(recv,MyGpsId,recv_length);
        
          if(PROTOCOL_TYPE_GPS_RESPONSE==commandtype){          
             // greenled_work(2);
             NRF_LOG_INFO("into LORA_REV_RESPOSE_DONE\r\n");
              locationcard_event=BATTERY_VOL_GET_EVT;//周期结束，采集一次电量
          }else if(PROTOCOL_TYPE_CFGGPSTIME==commandtype){
            NRF_LOG_INFO("into LORA_CONFIG\r\n");
            for(uint8_t i=0;i<4;i++){
                flash_store_data.flash_store_data_s.gpslooptime.arr[3-i]=recv[11+i];
            }        
            myflash_write(flash_store_data.flashdata,FLASHDATA_SIZE);
            gpslooptime= \
              ((uint32_t)flash_store_data.flash_store_data_s.gpslooptime.arr[2])*3600+ \
              ((uint32_t)flash_store_data.flash_store_data_s.gpslooptime.arr[1])*60+ \
              ((uint32_t)flash_store_data.flash_store_data_s.gpslooptime.arr[0]);
            if(gpslooptime<46){
             gpslooptime=1;
            }else{
            gpslooptime=gpslooptime-45;
            }
            response_message[0]=0x01;
            for(uint8_t j=0;j<4;j++){
              response_message[4-j]=recv[recv_length-3-j];
            }
            locationcard_event=LORA_SENDING_RESPONSE_EVT;//发送响应
            //保存时间间隔到结构体和flash;
          }        
           recv_length=0;   
           lorarecv_timeout=0;
        }else if( sx1278_tx_count>0){
          NRF_LOG_INFO("into LORA_REV_RESPOSE_GPS_TIMEOUT\r\n");
         // greenled_work(4);
          locationcard_event=GPS_REV_DONE_EVT;//重新发
        }else if( sx1278_tx_vol_count>0){
          NRF_LOG_INFO("into LORA_REV_RESPOSE_VOL_TIMEOUT\r\n");
           locationcard_event=BATTERY_VOL_LOW;//重新发
        }
         lorarecv_timeout=0;
      //}
      break;
    case LORA_SENDING_RESPONSE_EVT: 
      NRF_LOG_INFO("into LORA_SENDING_RESPONSE_EVT\r\n");
        gps_close();
        lora_close();
        nrf_drv_wdt_channel_feed(m_channel_id);
        lora_open();
        Sx1278_Init(471500000);
        MakeUpCommand(response_message,5,PROTOCOL_TYPE_CFG_RESPONSE);
        SxSend(proto_data_str,0,proto_data_len);//max=56.6mA    
        locationcard_event=LOCATIONCARD_GPS_START_EVT;     
     break;
    case GPS_REV_DONE_EVT:
       NRF_LOG_INFO("into GPS_REV_DONE_EVT\r\n");
      if(sx1278_tx_count<2){
        gps_close();
        lora_close();
        nrf_drv_wdt_channel_feed(m_channel_id);
        lora_open();
        Sx1278_Init(471500000);
        MakeUpCommand(gps_str,gps_str_ptr,PROTOCOL_TYPE_GPS);
        SxSend(proto_data_str,0,proto_data_len);//max=56.6mA
        sx1278_tx_count++;
        Sx1278_Init(471300000);
        locationcard_event=LORA_REV_READY_EVT;
      }else{
        sx1278_tx_count=0;
        locationcard_event=BATTERY_VOL_GET_EVT;//周期结束采集电量
      }
     break;
    case LOCATIONCARD_GPS_START_EVT:
       NRF_LOG_INFO("into LOCATIONCARD_GPS_START_EVT\r\n");
      lora_close();
      sx1278_tx_count=0;
      sx1278_tx_vol_count=0;
      gps_str_ptr=0;
      proto_data_len=0;          
      location_timers_start();//Max=1.43mA  
      nrf_drv_wdt_channel_feed(m_channel_id);
      locationcard_event=LOCATIONCARD_IDEL_EVT;
      break;
    case SOS_OCCURS_EVT:     
      NRF_LOG_INFO("into SOS_OCCURS_EVT\r\n");
      nrf_delay_ms(4000); 
      nrf_drv_wdt_channel_feed(m_channel_id);
      if(button_count==3){
        location_timers_stop();
        gps_close();  
        lora_close();
        proto_data_len=0; 
        MakeUpCommand(sos_message,4,PROTOCOL_TYPE_SOS);
        lora_open();
        Sx1278_Init(471500000);
        SxSend(proto_data_str,0,proto_data_len);//max=56.6mA
        proto_data_len=0;
      }else if(button_count==5){
         blueled_work(3);        
         NVIC_SystemReset();      
      }
      locationcard_event=LOCATIONCARD_GPS_START_EVT;
      button_count=0;
      break;
    case BATTERY_VOL_LOW:
      NRF_LOG_INFO("into BATTERY_VOL_LOW\r\n");
      if(sx1278_tx_vol_count<2){
        gps_close();
        lora_close();
        proto_data_len=0;
        nrf_drv_wdt_channel_feed(m_channel_id);
        MakeUpCommand(battery_vol_value,1,PROTOCOL_TYPE_POWER_LOW);
        lora_open();
        Sx1278_Init(471500000);
         sx1278_tx_vol_count++;
         sx1278_tx_count=0;
        SxSend(proto_data_str,0,proto_data_len);//max=56.6mA
         Sx1278_Init(471300000);
        proto_data_len=0;
        locationcard_event=LORA_REV_READY_EVT;
      }else{
        sx1278_tx_vol_count=0;
        locationcard_event=LOCATIONCARD_GPS_START_EVT;//周期结束开始gps信息采集
      }
      break;
    case LOCATIONCARD_IDEL_EVT:
    //  NRF_LOG_INFO("into LOCATIONCARD_IDEL_EVT\r\n");
      
        if(battery_vol_value[0]==0x1b){
          blueled_work(1);
        }
     
        break;
    }
     if(stu_bq25070_read()==0){//充电进行中
     
      redled_open();
    }else//充满或者未充电
    {
      redled_close();
    }
    
  }
}




/**
* @}
*/

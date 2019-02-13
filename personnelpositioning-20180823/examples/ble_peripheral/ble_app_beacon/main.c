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
//#include "bsp.h"
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
#define CENTRAL_LINK_COUNT              0                                 /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           0                                 /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                 /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define APP_CFG_NON_CONN_ADV_TIMEOUT    0                                 /**< Time for which the device must be advertising in non-connectable mode (in seconds). 0 disables timeout. */
#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(500, UNIT_0_625_MS) /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */

#define DEAD_BEEF                       0xDEADBEEF                        /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define APP_TIMER_PRESCALER             0                                 /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                 /**< Size of timer operation queues. */

#define DEVICE_NAME                     "UBIPP-10004"                               /**< Name of device. Will be included in the advertising data. */
uint8_t gps_timeout=0;
static ble_gap_adv_params_t m_adv_params;                                 /**< Parameters to be passed to the stack when starting advertising. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */

uint8_t gps_finish_flag=0;
uint8_t gps_str_ptr=0;
uint8_t gps_str[256]={0};
uint8_t gps_str_gsa[256];
void gps_close(void);
uint8_t TimeSeconds=0;
uint8_t TimeMinute=59;
uint8_t TimeHour=8;
unsigned char teststr[4]={"3333"};
uint8_t MyGpsId[8]={0x83,0x60,0x18,0x10,0x03,0x12,0x00,0x04};
APP_TIMER_DEF(m_location_timer_id);
APP_TIMER_DEF(m_rtc_timer_id);
APP_TIMER_DEF(m_adc_timer_id);
nrf_drv_wdt_channel_id m_channel_id;  
uint8_t cmd_number=0;
uint8_t mode=0;
uint8_t seg_count=0;		//逗号计数加1
uint8_t byte_count=0;
//GPS数据存储数组
uint8_t JD[10];		//经度
uint8_t JD_a;		//经度方向
uint8_t WD[9];		//纬度
uint8_t WD_a;		//纬度方向
uint8_t date[6];		//日期
uint8_t time[6];		//时间
uint8_t time1[6];		//时间
uint8_t speed[5]={'0','0','0','0','0'};		//速度
uint8_t hdop[4];		//高度
uint8_t angle[5];		//方位角
uint8_t use_sat[2];	//使用的卫星数
uint8_t total_sat[2];	//天空中总卫星数
uint8_t lock;			//定位状态
uint8_t pdop[4];
uint8_t gps_str_gsa_ptr=0;
uint8_t gps_str_gsa[256];
typedef enum {
 Start= 0,// $,
 G1 =1,
 N =2,
 G2= 3,
 G3 =4,
 A =5 ,
 Data =6 ,
 Check0=7,
 Check1=8,
 GPSLOCK=9,
 GPSHDOP=10,
 SECOND=11,//$
 G4=12,
 N2=13,
 G5=14,
 S=15,
 A2=16,
 Data2=17,
 Check2=18,
 Check3=19,
 GPSPDOP=20

}GPS_STATE;
uint8_t  num_lock=0;

uint8_t gpscount=0;
uint32_t gpstimeout=6550;//25250;//29250;//40秒//8550;//10秒58500;//60秒//8550;//10秒
uint8_t timeout=0;

uint8_t sos_flag=0;
uint8_t sos_message[4]={0x01,0x01,0x01,0x01};
uint8_t button_count=0;
#define APP_GPIOTE_MAX_USERS                 1  // Maximum number of users of the GPIOTE handler.
uint8_t sendmessage[16]={"Failed to locate"};
#define TIME_LEVEL_MEAS_INTERVAL APP_TIMER_TICKS(gpstimeout,APP_TIMER_PRESCALER)
#define TIME_LEVEL_RTC_INTERVAL APP_TIMER_TICKS(1000,APP_TIMER_PRESCALER)
#define TIME_LEVEL_ADC_INTERVAL APP_TIMER_TICKS(10000,APP_TIMER_PRESCALER)
bool worktime=false;
uint8_t firsttime=0;
//typedef enum
//{
//    LORA_SEND_DONE_EVT,    
//    LORA_REV_DONE_EVT,
//    GPS_REV_DONE_EVT,
//    LOCATIONCARD_IDLE_EVT,
//} locationcard_evt_type_t;
locationcard_evt_type_t locationcard_event; 

/** ----------------------------------------------adc--------------------------------------------------------*/

#define ADC_BUFFER_SIZE 1                               /**< Size of buffer for ADC samples.  */
nrf_adc_value_t       adc_buffer[ADC_BUFFER_SIZE]; /**< ADC buffer. */
static nrf_drv_adc_channel_t m_channel_config = NRF_DRV_ADC_DEFAULT_CHANNEL(NRF_ADC_CONFIG_INPUT_2); /**< Channel instance. Default configuration used. */
uint8_t adc_buff[10]={0};
uint8_t adc_index=0;
nrf_adc_value_t       adc_value;

void lora_close(void){
  nrf_gpio_cfg_output(CTL_LR_VDD);
  nrf_gpio_pin_set(CTL_LR_VDD);
}
void lora_open(void){
  nrf_gpio_cfg_output(CTL_LR_VDD);
  nrf_gpio_pin_clear(CTL_LR_VDD);
}

/** ----------------------------------------------adc end--------------------------------------------------------*/

/**@brief Callback function for asserts in the SoftDevice.
*
* @details This function will be called in case of an assert in the SoftDevice.
*
* @warning This handler is an example only and does not fit a final product. You need to analyze
*          how your product is supposed to react in case of Assert.
* @warning On assert from the SoftDevice, the system can only recover on reset.
*
* @param[in]   line_num   Line number of the failing ASSERT call.
* @param[in]   file_name  File name of the failing ASSERT call.
*/
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
  
  //err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
  //APP_ERROR_CHECK(err_code);
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
}

/**@brief   Function for handling app_uart events.
*
* @details This function will receive a single character from the app_uart module and append it to
*          a string. The string will be be sent over BLE when the last character received was a
*          'new line' i.e '\r\n' (hex 0x0D) or if the string has reached a length of
*          @ref NUS_MAX_DATA_LENGTH.
*/
/**@snippet [Handling the data received over UART] */
GPS_STATE gps_sta=Start;
double d_hdop=0.0;
void uart_event_handle(app_uart_evt_t * p_event)
{   
  static uint8_t crc = 0;    
  uint8_t data = 0;
  
  //$GPRMC,144601.00,A,3916.72973,N,11706.60267,E,0.719,,180117,,,A*76
  if (p_event->evt_type==APP_UART_DATA_READY) 
  {
    UNUSED_VARIABLE(app_uart_get(&data));             
    switch(gps_sta) // find GNGGA
    {
    case Start:
      if(data == '$') {
        gps_sta = G1;
        gps_str_ptr = 0;                   
        memset(gps_str,0,sizeof(gps_str));                  
        gps_str[gps_str_ptr++]=data;
        crc = 0;
      }
      else gps_sta = Start; 
      break;
    case G1:
      if(data == 'G'){
        gps_sta = N;
        gps_str[gps_str_ptr++]=data;
        crc ^= data;
      }
      else gps_sta = Start; 
      break;
    case N:
      if(data == 'N'){
        gps_sta = G2;
        gps_str[gps_str_ptr++]=data;
        crc ^= data;
      }
      else gps_sta = Start; 
      break;
    case G2:
      if(data == 'G'){
        gps_sta = G3;
        gps_str[gps_str_ptr++]=data;
        crc ^= data;
      }
      else gps_sta = Start; 
      break;
    case G3:
      if(data == 'G'){
        gps_sta = A;
        gps_str[gps_str_ptr++]=data;
        crc ^= data;
      }
      else gps_sta = Start; 
      break;
    case A:
      if(data == 'A'){
        gps_sta = Data;
        gps_str[gps_str_ptr++]=data;
        crc ^= data;
      }
      else gps_sta = Start; 
      break;
    case Data:
      if(data == '*'){ 
        gps_sta = Check0;
        gps_str[gps_str_ptr++]=data;
      }else if(data==','){
        gps_sta = Data;
        gps_str[gps_str_ptr++]=data;
        crc ^= data;       
        seg_count++;
        if(seg_count==6){
          gps_sta = GPSLOCK;
        }
//        if(seg_count==8){
//          gps_sta = GPSHDOP;
//        }
      }
      else{
        gps_sta = Data;
        gps_str[gps_str_ptr++]=data;
        crc ^= data;
        if(gps_str_ptr>127) gps_sta = Start; 
      }
      break;
    case GPSLOCK: 
       gps_sta = Data;
       gps_str[gps_str_ptr++]=data;
       crc ^= data;
       lock=data;
       break;
//    case GPSHDOP: 
//     
//       hdop[byte_count++]=data;
//       // gps_sta = GPSHDOP;
//       if(byte_count>3){
//         gps_sta = Data;
//        d_hdop=atof((char const*)hdop);
//        byte_count=0;
//       }
//       //gps_sta = Data;
//       gps_str[gps_str_ptr++]=data;
//       crc ^= data;
//       break;
    case Check0:
      gps_sta = Check1;
      gps_str[gps_str_ptr++]=data;
      break;
    case Check1: //*hh
      gps_sta = Start;
      gps_str[gps_str_ptr++]=data;
      if(crc == ((gps_str[gps_str_ptr-2]-'0')*16 + (gps_str[gps_str_ptr-1]-'0')))
      {
       // if(lock=='1'){
        //  num_lock++;
          lock=0;
         // if(num_lock==1){//||(0.5<=d_hdop&&d_hdop<=1.5)){
            //num_lock=0;
            //d_hdop=0;
           // gps_close();
            //gps_str[gps_str_ptr++] = '\r';
            //gps_str[gps_str_ptr] = '\n';
           gps_sta=SECOND;
            //gps_finish_flag=0x04; 
          //}
        //}       
      //}
    }  
    break;
    case SECOND:
      if(data == '$') {
        gps_sta = G4;
        gps_str_gsa_ptr = 0;                   
        memset(gps_str_gsa,0,sizeof(gps_str_gsa));                  
        gps_str_gsa[gps_str_gsa_ptr++]=data;
        crc = 0;
      }
      else gps_sta = SECOND; 
    break;
    case G4:
    if(data == 'G'){
        gps_sta = N2;
        gps_str_gsa[gps_str_gsa_ptr++]=data;
        crc ^= data;
      }
      else gps_sta = SECOND; 
      break;
     case N2:
    if(data == 'N'){
        gps_sta = G5;
        gps_str_gsa[gps_str_gsa_ptr++]=data;
        crc ^= data;
      }
      else gps_sta = SECOND; 
      break;
     case G5:
     if(data == 'G'){
        gps_sta = S;
        gps_str_gsa[gps_str_gsa_ptr++]=data;
        crc ^= data;
      }
      else gps_sta = SECOND; 
      break;
      case S:
      if(data == 'S'){
        gps_sta = A2;
        gps_str_gsa[gps_str_gsa_ptr++]=data;
        crc ^= data;
      }
      else gps_sta = SECOND; 
      break;
      case A2:
      if(data == 'A'){
        gps_sta = Data2;
        gps_str_gsa[gps_str_gsa_ptr++]=data;
        crc ^= data;
      }
      else gps_sta = SECOND; 
      break;
      case Data2:
      if(data == '*'){ 
        gps_sta = Check3;
        gps_str_gsa[gps_str_gsa_ptr++]=data;
      }else if(data==','){
        gps_sta = Data2;
        gps_str_gsa[gps_str_gsa_ptr++]=data;
        crc ^= data;       
        seg_count++;
        if(seg_count==15){
          gps_sta = GPSPDOP;
        }
      }
      else{
        gps_sta = Data2;
        gps_str_gsa[gps_str_gsa_ptr++]=data;
        crc ^= data;
        if(gps_str_gsa_ptr>127) gps_sta = SECOND; 
      }
      break;
     case GPSPDOP:    
       pdop[byte_count++]=data;
       if(byte_count>3){
         gps_sta = Data2;
        d_hdop=atof((char const*)pdop);
        byte_count=0;
       }
       gps_str_gsa[gps_str_gsa_ptr++]=data;
       crc ^= data;
       break;
    case Check2:
      gps_sta = Check3;
      gps_str_gsa[gps_str_gsa_ptr++]=data;
      break;
    case Check3: //*hh
      gps_sta = Start;
      gps_str_gsa[gps_str_gsa_ptr++]=data;
      if(crc == ((gps_str_gsa[gps_str_gsa_ptr-2]-'0')*16 + (gps_str_gsa[gps_str_gsa_ptr-1]-'0')))
      {
       // if(lock=='1'){
        //  num_lock++;
          lock=0;
         // if(num_lock==1){//||(0.5<=d_hdop&&d_hdop<=1.5)){
            //num_lock=0;
            //d_hdop=0;
           // gps_close();
            //gps_str[gps_str_ptr++] = '\r';
            //gps_str[gps_str_ptr] = '\n';
           //gps_sta=SECOND;
            gps_finish_flag=0x04; 
          //}
        //}       
      //}
    }  
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
 // app_uart_close();
}
void gps_open(void){
  //uart_init();
  NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
  nrf_gpio_cfg_output(CTL_GPS_VDD);
  nrf_gpio_pin_clear(CTL_GPS_VDD);
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
 
    location_timers_stop(); 
    NRF_LOG_INFO("location_timers_stop\r\n");
    gps_open();  
    NRF_LOG_INFO("gps_open\r\n");
}
void location_timers_start(void)
{
  uint32_t err_code;
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
void adc_start(void){
  
  APP_ERROR_CHECK(nrf_drv_adc_buffer_convert(&adc_value,ADC_BUFFER_SIZE));
  // manually trigger ADC conversion
  nrf_drv_adc_sample();                       
}
static void adc_timeout_handler(void * p_context)
{
  UNUSED_PARAMETER(p_context);
  adc_start(); 
}
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
  blueled_work();
 // NVIC_SystemReset();
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
/**
* @brief ADC interrupt handler.
*/
static void adc_event_handler(nrf_drv_adc_evt_t const * p_event)
{
  if (p_event->type == NRF_DRV_ADC_EVT_DONE)
  {       
    if(p_event->data.done.p_buffer[0]<720){
      blueled_work();
    }else{
      blueled_close();
    }
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
  timers_init();
  ble_stack_init();
  gap_params_init();
  advertising_init();
  advertising_start();
 // lora_open();
  //mylorainit(); 
  // wdt_init();
  uart_init();
  spi_init();
  button_init();
  location_timers_start();
  
  //adc_config();
  
  for (;; )
  {  
    if(sos_flag==1){
      gps_close();
      nrf_delay_ms(3000);
      nrf_drv_wdt_channel_feed(m_channel_id);
      if(button_count==3){   
        sos_flag=0;
        proto_data_len=0;
        lora_open();
        nrf_delay_ms(10);
        mylorainit();
        MakeUpCommand(sos_message,4,PROTOCOL_TYPE_GPS);
        SxSend(proto_data_str,0,proto_data_len);//max=56.6mA
        lora_close();
        proto_data_len=0;
        
        
      }else if(button_count==5){
         blueled_work();     
         NVIC_SystemReset();       
      }
      gps_open();
      button_count=0;
      sos_flag=0;
    }
    if(gps_finish_flag==0x04){           
      //gps_close();
      lora_open();
     mylorainit();
      MakeUpCommand(gps_str,gps_str_ptr,PROTOCOL_TYPE_GPS);
      SxSend(proto_data_str,0,proto_data_len);//max=56.6mA
      lora_close();
      gps_finish_flag=0;
      gps_str_ptr=0;
      proto_data_len=0;          
      location_timers_start();//Max=1.43mA  
      nrf_drv_wdt_channel_feed(m_channel_id);           
    }   
    if(stu_bq25070_read()==0){
      redled_open();
    }else
    {
      redled_close();
    }
  }
}


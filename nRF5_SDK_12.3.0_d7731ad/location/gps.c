#include "gps.h"
#include "nrf_gpio.h"
#include "pca10028.h"
#include "app_uart.h"

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */
uint8_t gps_finish_flag=0;
uint8_t gps_timeout=0;
uint8_t gps_str[256];
uint8_t gps_str_ptr=0;


//*******************************************
/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' i.e '\r\n' (hex 0x0D) or if the string has reached a length of
 *          @ref NUS_MAX_DATA_LENGTH.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t * p_event)
{
   
    //static uint8_t index = 0;
   // uint32_t       err_code;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&gps_str[gps_str_ptr]));
           // gps_str_ptr++;
            
            if(gps_finish_flag==0x04){
              return;
            }
            
            if(gps_finish_flag==0x00){
              if(gps_str[gps_str_ptr]=='$'){
                gps_finish_flag=0x01;
                gps_str_ptr=0;
              }
            }
            if(gps_finish_flag==0x01){
                if(gps_str_ptr>=255){
                  gps_finish_flag=0x00;
                  gps_str_ptr=0;
                }               
                //GNRMC
                if(gps_str_ptr==6){
                    if(gps_str[1]=='G'&&gps_str[2]=='N'&&gps_str[3]=='R'&&gps_str[4]=='M'&&gps_str[5]=='C'){
                            gps_finish_flag=0x01;
                    }else{
                            gps_finish_flag=0x00;
                            gps_str_ptr=0;
                            return;
                    }
                }
                if(gps_str[gps_str_ptr]=='*'){
                    if(gps_str[gps_str_ptr-1]=='A'){//||gps_timeout>120){
                        gps_timeout=0;
                        gps_finish_flag=0x04;
                        NRF_UART0->INTENSET = UART_INTENSET_RXDRDY_Disabled << UART_INTENSET_RXDRDY_Pos;
                    }
                        //                    }else{
//                        gps_timeout++;
//                        gps_str_ptr=0;
//                            
//                    }
                }            
                gps_str_ptr++;               
            }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}
/**@snippet [Handling the data received over UART] */
//*******************************************
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
/**@snippet [UART Initialization] */

//*******************************************
void OpenGps(void){
  
  nrf_gpio_pin_clear(CTL_GPS_VDD);
  uart_init();//enable uart
}
//*******************************************
void CloseGps(void){
  
  app_uart_close();//disable uart 
  nrf_gpio_pin_set(CTL_GPS_VDD);
}
//********************************************
void GpsInit(void){
  nrf_gpio_cfg_output(CTL_GPS_VDD);
  CloseGps();
  nrf_gpio_cfg_output(RX_PIN_NUMBER);
  nrf_gpio_pin_clear(RX_PIN_NUMBER);

}
//*******************************************
void ParseGps(void);




#ifndef MAIN_H
#define MAIN_H

#define CTL_GPS_VDD    11
#define RX_PIN_NUMBER  10
#define TX_PIN_NUMBER  
#define CTS_PIN_NUMBER 
#define RTS_PIN_NUMBER 
#define HWFC           false

#define SPI_PSELMISO0  30    // SPI MISO signal.
#define SPI_PSELSS0    28    // SPI CSN signal.
#define SPI_PSELMOSI0  29    // SPI MOSI signal.
#define SPI_PSELSCK0   00    // SPI SCK signal.

#define BLUE_LED       18
#define GREEN_LED      19
#define RED_LED        20

#define CTL_LR_VDD     21
#define LORA_CTRL      25
#define LORA_RESET	7
#define DIO0            6

#define button_1        22
#define STU_BQ25070     23

typedef enum
{
    LORA_SEND_DONE_EVT,    
    LORA_REV_DONE_EVT,
    GPS_REV_DONE_EVT,
    SOS_OCCURS_EVT,
    LOCATIONCARD_GPS_START_EVT,
    GPS_REV_FAIL_EVT,
    LORA_REV_READY_EVT,
    LOCATIONCARD_IDEL_EVT,
    LORA_SENDING_RESPONSE_EVT,
    BATTERY_VOL_LOW,
    BATTERY_VOL_GET_EVT,
} locationcard_evt_type_t;
void power_manage(void);

#endif
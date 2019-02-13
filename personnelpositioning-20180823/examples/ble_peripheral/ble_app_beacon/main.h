#ifndef MAIN_H
#define MAIN_H

#define CTL_GPS_VDD    11
#define RX_PIN_NUMBER  10
#define TX_PIN_NUMBER  NULL
#define CTS_PIN_NUMBER NULL
#define RTS_PIN_NUMBER NULL
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
    LOCATIONCARD_IDLE_EVT,
    GPS_REV_FAIL_EVT,
} locationcard_evt_type_t;
#endif
#ifndef MAIN_H
#define MAIN_H
void power_manage(void);
typedef enum
{
    LORA_SEND_DONE_EVT,    
    LORA_REV_DONE_EVT,
    GPS_REV_DONE_EVT,
    SOS_OCCURS_EVT,
    LOCATIONCARD_IDLE_EVT,
} locationcard_evt_type_t;
#endif
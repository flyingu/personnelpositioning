#ifndef __BATTERY__H__
#define __BATTERY__H__
void lpcomp_init(void);
void saadc_init(void);
void saadc_sampling_event_enable(void);
void saadc_sampling_event_init(void);
void adc_config(void);
#endif
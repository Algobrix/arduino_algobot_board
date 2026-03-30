#ifndef __DELAY_H_
#define __DELAY_H_

#include <Arduino.h>

#define SYSTIM_TIMEOUT							1
#define SYSTIM_KEEP_ALIVE						0
 
uint32_t 	getSYSTIM(void);
uint8_t 	chk4TimeoutSYSTIM(uint32_t btime, uint32_t period);	
uint32_t 	getElapsedTimeSYSTIM(uint32_t t_beg);		

extern volatile uint32_t gu32Jiffies1ms;

#endif 

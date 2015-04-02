#ifndef __TIME_H
#define __TIME_H
#include <stdint.h>
#include <avr/io.h>

#include "pins.h"

#define TIME_INTERVALL 1
#define TIME_DEBOUNCE_INTERVALL 10



extern uint8_t time_setup(void);

extern uint32_t time_getMS(void);
extern uint32_t time_getSec(void);
extern uint32_t time_getDifference(uint32_t start, uint32_t end);

extern void time_delayMS(uint32_t delay);

#endif
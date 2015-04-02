#ifndef __TRIGGER_H
#define __TRIGGER_H
#include <stdint.h>
#include <avr/io.h>

#include "pins.h"


extern uint8_t trigger_setup(void);

extern void trigger_set(uint8_t camera, uint8_t flash, uint32_t delay, uint8_t bulbMode, uint32_t bulbDelay);
extern void trigger_trigger(void);
extern void trigger_setTriggers(uint8_t camera, uint8_t flash, uint8_t focus);
extern void trigger_reset(void);

#endif
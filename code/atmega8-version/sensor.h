#ifndef __SENSOR_H
#define __SENSOR_H
#include <stdint.h>
#include <avr/io.h>

#include "pins.h"

extern uint8_t sensor_setup(void);

extern uint16_t sensor_read(uint8_t channel);
extern void sensor_setTrigger(uint8_t channel, uint16_t threshold, uint8_t triggerOnLow);
extern void sensor_abortTrigger(void);
extern uint8_t sensor_wasTriggered(void);
extern uint16_t sensor_getTriggeredValue(void);

extern void sensor_resetTrigger(void);

#endif
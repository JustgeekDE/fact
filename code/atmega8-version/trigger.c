#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#include "pins.h"
#include "time.h"


uint8_t trigger_bulbMode = 0;
uint32_t trigger_bulbModeDelay = 0;
uint32_t trigger_delay = 0;
uint8_t trigger_flash = 1;
uint8_t trigger_camera = 1;

uint32_t trigger_bulbModeDelayStored = 0;
uint32_t trigger_delayStored = 0;


uint8_t trigger_setup(void){
	TRIGGER_DDR |= (1 << TRIGGER_SHUTTER_BIT) | (1 << TRIGGER_FLASH_BIT) | (1 << TRIGGER_FOCUS_BIT);
	return 1;

}

void trigger_setTriggers(uint8_t camera, uint8_t flash, uint8_t focus){
	uint8_t triggerMask = 0;
	uint8_t clearMask = 0;

	if(camera){
		triggerMask |= (1 << TRIGGER_SHUTTER_BIT);
	} else {
		clearMask |= (1 << TRIGGER_SHUTTER_BIT);
	}

	if(flash){
		triggerMask |= (1 << TRIGGER_FLASH_BIT);
	} else {
		clearMask |= (1 << TRIGGER_FLASH_BIT);
	}

	if(focus){
		triggerMask |= (1 << TRIGGER_FOCUS_BIT);
	} else {
		clearMask |= (1 << TRIGGER_FOCUS_BIT);
	}

	TRIGGER_PORT = (TRIGGER_PORT & ~clearMask) | triggerMask;
}

void trigger_set(uint8_t camera, uint8_t flash, uint32_t delay, uint8_t bulbMode, uint32_t bulbDelay){
	trigger_camera = camera;
	trigger_flash = flash;
	trigger_delay = delay;
	trigger_bulbMode = bulbMode;
	trigger_bulbModeDelay = bulbDelay;

	trigger_delayStored = delay;
	trigger_bulbModeDelayStored = bulbDelay;
	trigger_setTriggers(bulbMode,0,0);
}

void trigger_reset(void){
	trigger_delay = trigger_delayStored;
	trigger_bulbModeDelay = trigger_bulbModeDelayStored;
	trigger_setTriggers(trigger_bulbMode,0,0);
}

void trigger_trigger(void){
	while(trigger_delay){
		_delay_ms(1);
		trigger_delay--;
	}
	trigger_setTriggers(trigger_camera, trigger_flash, 0);
	
	if(trigger_bulbMode){
		while(trigger_bulbModeDelay){
			_delay_ms(1);
			trigger_bulbModeDelay--;
		}
	} else {
			_delay_ms(20);
		}
	trigger_setTriggers(0, 0, 0);
}



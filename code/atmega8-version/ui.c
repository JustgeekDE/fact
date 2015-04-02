#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#include "ui.h"
#include "pins.h"
#include "sensor.h"
#include "time.h"

volatile uint8_t ui_buttonStates[2];

int8_t ui_oldButtonStates[2];

uint8_t ui_setup(void) {
	// init pullups on buttons
	UI_BUTTON_PORT |= (1 << UI_BUTTON_ARM_BIT) | (1 << UI_BUTTON_MODE_BIT);

	// init LEDs
	UI_LED_DDR |= UI_LED_BITMASK;
	ui_oldButtonStates[UI_BUTTON_MODE] = UI_BUTTON_UP;
	ui_oldButtonStates[UI_BUTTON_ARM] = UI_BUTTON_UP;

  return 1;
}


void ui_setLedMask(uint8_t mask){
	UI_LED_PORT &= ~(UI_LED_BITMASK);

	if(mask & 0x01){
		SET_BIT(UI_LED_PORT,UI_LED_SOUND_GREEN_BIT);
	}
	if(mask & 0x02){
		SET_BIT(UI_LED_PORT,UI_LED_SOUND_RED_BIT);
	}

	if(mask & 0x04){
		SET_BIT(UI_LED_PORT,UI_LED_LIGHT_GREEN_BIT);
	}
	if(mask & 0x08){
		SET_BIT(UI_LED_PORT,UI_LED_LIGHT_RED_BIT);
	}

	if(mask & 0x10){
		SET_BIT(UI_LED_PORT,UI_LED_ARMED_GREEN_BIT);
	}
	if(mask & 0x20){
		SET_BIT(UI_LED_PORT,UI_LED_ARMED_RED_BIT);
	}
}

void ui_setLedMode(uint8_t mode, uint8_t armed){
	UI_LED_PORT &= ~(UI_LED_BITMASK);
	if(!armed){
		switch(mode){
			case 0:
				SET_BIT(UI_LED_PORT,UI_LED_SOUND_GREEN_BIT);
				break;

			case 1:
				SET_BIT(UI_LED_PORT,UI_LED_SOUND_RED_BIT);
				break;

			case 2:
				SET_BIT(UI_LED_PORT,UI_LED_LIGHT_GREEN_BIT);
				break;

			case 3:
				SET_BIT(UI_LED_PORT,UI_LED_LIGHT_RED_BIT);
				break;

			case 4:
				SET_BIT(UI_LED_PORT,UI_LED_SOUND_GREEN_BIT);
				SET_BIT(UI_LED_PORT,UI_LED_LIGHT_RED_BIT);
				SET_BIT(UI_LED_PORT,UI_LED_LIGHT_GREEN_BIT);
				break;

			case 5:
				SET_BIT(UI_LED_PORT,UI_LED_SOUND_RED_BIT);
				SET_BIT(UI_LED_PORT,UI_LED_LIGHT_RED_BIT);
				SET_BIT(UI_LED_PORT,UI_LED_LIGHT_GREEN_BIT);
				break;

			case 6:
				SET_BIT(UI_LED_PORT,UI_LED_LIGHT_GREEN_BIT);
				SET_BIT(UI_LED_PORT,UI_LED_SOUND_RED_BIT);
				SET_BIT(UI_LED_PORT,UI_LED_SOUND_GREEN_BIT);
				break;

			case 7:
				SET_BIT(UI_LED_PORT,UI_LED_LIGHT_RED_BIT);
				SET_BIT(UI_LED_PORT,UI_LED_SOUND_RED_BIT);
				SET_BIT(UI_LED_PORT,UI_LED_SOUND_GREEN_BIT);
				break;

			case 8:
				SET_BIT(UI_LED_PORT,UI_LED_LIGHT_RED_BIT);
				SET_BIT(UI_LED_PORT,UI_LED_LIGHT_GREEN_BIT);
				SET_BIT(UI_LED_PORT,UI_LED_SOUND_RED_BIT);
				SET_BIT(UI_LED_PORT,UI_LED_SOUND_GREEN_BIT);
				break;

			default:
				UI_LED_PORT |= (UI_LED_BITMASK);

		}
	}
}

void ui_setArmedLed(uint8_t red, uint8_t green){
	if(red) {
		SET_BIT(UI_LED_PORT,UI_LED_ARMED_RED_BIT);
	} else {
		CLEAR_BIT(UI_LED_PORT,UI_LED_ARMED_RED_BIT);
	}

	if(green) {
		SET_BIT(UI_LED_PORT,UI_LED_ARMED_GREEN_BIT);
	} else {
		CLEAR_BIT(UI_LED_PORT,UI_LED_ARMED_GREEN_BIT);
	}
}


// Button functions
int8_t ui_getButtonState(uint8_t button){
	uint8_t state = (ui_buttonStates[button] &0x0F);
	if(state == 0x0F) {
		return UI_BUTTON_UP;

	}
	if(state == 0x00) {
		return UI_BUTTON_DOWN;

	}
	return UI_BUTTON_BOUNCING;
}

int8_t ui_getButtonChange(uint8_t button){
	int8_t newState = ui_getButtonState(button);
	int8_t oldState = ui_oldButtonStates[button];

	if((newState == UI_BUTTON_BOUNCING) || (oldState == newState)){
		return UI_BUTTON_NOCHANGE;
	}
	ui_oldButtonStates[button] = newState;


	if(newState == UI_BUTTON_UP){
		return UI_BUTTON_RELEASED;
	}
	return UI_BUTTON_PRESSED;
}

uint16_t ui_getThreshold(void){
	return sensor_read(UI_POT_THRESH_CHANNEL);
}

uint32_t ui_getDelay(void){
	uint8_t factor = 0;

	uint16_t delay = (1023 - sensor_read(UI_POT_DELAY_CHANNEL));
	uint8_t jumperA = (sensor_read(UI_JUMPER_A_CHANNEL) != 0) ? 0x01 : 0x00;
	uint8_t jumperB = (sensor_read(UI_JUMPER_B_CHANNEL) != 0) ? 0x02 : 0x00;

	factor = (jumperA | jumperB);

	switch (factor) {
		case 0:
			delay = delay >> 3;
			break;

		case 1:
			delay = delay >> 1;
			break;

		case 2:
			delay = delay << 1;
			break;

		case 3:
			delay = delay << 3;
			break;

		default:
			break;
	}
	return delay;
}

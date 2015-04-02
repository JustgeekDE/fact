#ifndef __UI_H
#define __UI_H
#include <stdint.h>
#include <avr/io.h>

#include "pins.h"


#define UI_BUTTON_MODE 0
#define UI_BUTTON_ARM 1

#define UI_BUTTON_NOCHANGE 0
#define UI_BUTTON_PRESSED 1
#define UI_BUTTON_RELEASED -1

#define UI_BUTTON_BOUNCING 0
#define UI_BUTTON_DOWN -1
#define UI_BUTTON_UP 1



extern uint8_t ui_setup(void);

extern void ui_setLedMask(uint8_t mask);
extern void ui_setLedMode(uint8_t mode, uint8_t armed);
extern void ui_setArmedLed(uint8_t red, uint8_t green);

extern int8_t ui_getButtonState(uint8_t button);
extern int8_t ui_getButtonChange(uint8_t button);

extern uint16_t ui_getThreshold(void);
extern uint32_t ui_getDelay(void);

extern volatile uint8_t ui_buttonStates[2];

// Interrup Controller to be included in timer interrupt
#define  UI_DEBOUNCE_IRQ() do{ \
		ui_buttonStates[0]  = ui_buttonStates[0] << 1; \
		ui_buttonStates[1]  = ui_buttonStates[1] << 1; \
		ui_buttonStates[0] |= ((UI_BUTTON_PIN >> UI_BUTTON_MODE_BIT) & 0x01); \
		ui_buttonStates[1] |= ((UI_BUTTON_PIN >> UI_BUTTON_ARM_BIT) & 0x01); \
	} while(0)



#endif
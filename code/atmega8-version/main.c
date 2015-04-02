#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <stdlib.h>

#include "pins.h"
#include "uart.h"
#include "ui.h"
#include "time.h"
#include "sensor.h"
#include "trigger.h"

#define DEBUG 0
#define EXTENDED_MODES 1
#define TOTAL_MODES 8
#define LONG_PRESS_DURATION 750
#define UI_UPDATE_INTERVALL 50

inline uint8_t triggerDirectionFromMode(uint8_t mode){
	// first bit is direction;
	return (mode & 0x01) ? 1 : 0;
}

inline uint8_t sensorChannelFromMode(uint8_t mode){
	// second bit is channel;
	return ((mode & 0x02) ? SENSOR_LIGHT_CHANNEL : SENSOR_SOUND_CHANNEL);
}

inline uint8_t bulbSettingFromMode(uint8_t mode){
	// third bit is bulb;
	return ((mode & 0x04) ? 1 : 0);
}

void printDebug(uint8_t mode, uint8_t continousMode){
	char buffy[48];
	uint8_t triggerOnLow =  triggerDirectionFromMode(mode);
	uint8_t channel = sensorChannelFromMode(mode);
	uint8_t bulbMode = bulbSettingFromMode(mode);

	if(mode == 8){
		uart_puts("\n\rTriggering every ");
		utoa( ui_getDelay(), buffy, 10 );
		uart_puts(buffy);
		uart_puts(" seconds.");
	} else {
		char* channelString = (channel == SENSOR_SOUND_CHANNEL) ? "Sound" : ((channel == SENSOR_LIGHT_CHANNEL ? "light" : "invalid"));

		uart_puts("\n\rMonitoring: ");
		uart_puts(channelString);

		uart_puts("\n\rTriggering ");
		uart_puts((continousMode ? "continously" : "once"));

		uart_puts(" on ");
		uart_puts((triggerOnLow ? "low" : "high"));

		uart_puts("\n\rBulb mode is ");
		uart_puts((bulbMode ? "on" : "off"));

		uart_puts("\n\rThreshold: ");
		utoa( ui_getThreshold(), buffy, 10 );
		uart_puts(buffy);

		uart_puts("\t\tDelay: ");
		utoa( ui_getDelay(), buffy, 10 );
		uart_puts(buffy);

		uart_puts("\n\rCurrent value is: ");
		utoa( sensor_read(channel), buffy, 10 );
		uart_puts(buffy);
	}
}

int main(void) {
	wdt_disable();
	cli();
	trigger_setup();
	ui_setup();
	uart_setup();
	time_setup();
	sensor_setup();
	sei();


	uart_puts("\n\n\r***************************************");
	uart_puts("\n\r*                                     *");
	uart_puts("\n\r*  simple FACT, simple firmware v0.2  *");
	uart_puts("\n\r*                                     *");
	uart_puts("\n\r***************************************");

	char buffy[48];

	uint8_t mode = 0;
	uint8_t armed = 0;
	uint16_t threshold = 0;
	uint8_t armDecay = 0;

	uint8_t triggerOnLow =  triggerDirectionFromMode(mode);
	uint8_t channel = sensorChannelFromMode(mode);
	uint8_t bulbMode = bulbSettingFromMode(mode);
	uint8_t continousMode = 0;
	uint32_t delay = 0;

	uint32_t lastUIUpdate = 0;
	uint32_t armPressed = 0;
	uint32_t lastTrigger = 0;
	uint32_t triggerIntervall = 0;

	ui_setLedMode(mode, armed);

	while(1){
		if(armed){
			// check if we should abort
			if(ui_getButtonState(UI_BUTTON_ARM) == UI_BUTTON_DOWN){
				ui_setArmedLed(0, 1);
			}

			if(mode == 8){
				// intervall trigger
				if(time_getDifference(lastTrigger, time_getMS()) >= triggerIntervall){
					trigger_setTriggers(1,1,0);
					lastTrigger = time_getMS();
					time_delayMS(100);
					trigger_setTriggers(0,0,0);

					utoa( lastTrigger, buffy, 10 );
					uart_puts(buffy);
					uart_puts(", ");
				}
			} else {
				if(sensor_wasTriggered()){
					uart_puts("\n\r+++ Triggered with value: ");
					utoa( sensor_getTriggeredValue(), buffy, 10 );
					uart_puts(buffy);
					uart_puts(" +++\n");

					if(continousMode){
						if(bulbMode){
							time_delayMS(100);
						}
						uart_puts("\n\r\tRearming...");
						ui_setLedMode(mode, armed); // disable LEDs before setting triggers
						trigger_reset();
						sensor_resetTrigger();
					} else {
						armed = 0;
						trigger_setTriggers(0,0,0);
						ui_setLedMode(mode, armed);
					}
				}
			}

			if((ui_getButtonChange(UI_BUTTON_MODE) == UI_BUTTON_RELEASED) || (ui_getButtonChange(UI_BUTTON_ARM) == UI_BUTTON_RELEASED)){
				uart_puts("\n\r--- Aborted! ---\n");
				// abort
				armed = 0;
				sensor_abortTrigger();
				trigger_setTriggers(0,0,0);
				ui_setLedMode(mode, armed);
			}

		} else {
			// If not armed to various checks
			int8_t modeButtonChange = ui_getButtonChange(UI_BUTTON_MODE);
			int8_t armButtonChange = ui_getButtonChange(UI_BUTTON_ARM);
			int8_t modeButtonState = ui_getButtonState(UI_BUTTON_MODE);
			int8_t armButtonState = ui_getButtonState(UI_BUTTON_ARM);


			if(modeButtonChange == UI_BUTTON_RELEASED){
				mode++;
//				mode = mode & TOTAL_MODES;
				while(mode > TOTAL_MODES){
					mode -= TOTAL_MODES;
				}
				armDecay = 0;
				triggerOnLow =  triggerDirectionFromMode(mode);
				channel = sensorChannelFromMode(mode);
				bulbMode = bulbSettingFromMode(mode);

				uart_puts("\n");
				printDebug(mode, 0);
				lastUIUpdate = time_getMS();

			}
			
			if(armButtonChange == UI_BUTTON_PRESSED){
				ui_setArmedLed(1, 0);
				armPressed = time_getMS();
			}

			if(time_getDifference(lastUIUpdate, time_getMS()) > UI_UPDATE_INTERVALL){
				lastUIUpdate = time_getMS();
				threshold = ui_getThreshold();
				if(armDecay){
					armDecay--;
				}
			}

			uint16_t value = sensor_read(channel);
			if(triggerOnLow && (value < threshold)){
				armDecay = 5;
			}
			if(!triggerOnLow && (value > threshold)){
				armDecay = 5;
			}
			ui_setLedMode(mode, armed);

			if(armButtonState == UI_BUTTON_DOWN){
				if(time_getDifference(armPressed, time_getMS()) > LONG_PRESS_DURATION){
					ui_setArmedLed(1, 1);
				}
			} else {
				ui_setArmedLed(armDecay, 0);
			}



			if(armButtonChange == UI_BUTTON_RELEASED){
				armed = 1;
				if(time_getDifference(armPressed, time_getMS()) > LONG_PRESS_DURATION){
					continousMode = 1;
				} else {
					continousMode = 0;
				}

				delay = ui_getDelay();
				ui_setLedMode(mode, armed); // disable LEDs before setting triggers

				uart_puts("\n\n\r +++ Arming +++");
				printDebug(mode, continousMode);

				if(mode == 8){
					triggerIntervall = delay * 1000;
					lastTrigger = time_getMS();
				} else {
					trigger_set(1, 1, delay, bulbMode, 1);
					sensor_setTrigger(channel, threshold, triggerOnLow);
				}
			}
		}
	}
}
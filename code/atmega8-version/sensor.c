#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "sensor.h"
#include "pins.h"
#include "trigger.h"

volatile uint8_t sensor_freeRunning = 0;
uint8_t sensor_triggerOnLow = 0;
uint16_t sensor_threshold = 0;
uint8_t sensor_channel = 0;

volatile uint16_t sensor_triggeredValue = 0;

#define SENSOR_ADC_START() (ADCSRA |= (1 << ADSC))

inline void sensor_selectChannel(uint8_t channel){
  // wait for current read to end
  while (ADCSRA & (1<<ADSC));
  // Select channel
  ADMUX = (ADMUX & ~(0x0F)) | (channel & 0x0F);
}

inline void sensor_setIRQ(uint8_t enable){
  if(enable){
    ADCSRA |= (1 << ADIE) | (1 << ADATE); 
  } else {
    ADCSRA &= ~((1 << ADIE) | (1 << ADATE)); 
  }
}


ISR(ADC_vect){
  uint16_t result = ADCW;
  if(((result < sensor_threshold) && (sensor_triggerOnLow != 0)) || ((result > sensor_threshold) && (sensor_triggerOnLow == 0))) {
    sensor_setIRQ(0);
    trigger_trigger();
    sensor_triggeredValue = result;
    sensor_freeRunning = 0;
  }
}

uint8_t sensor_setup(void){
  // enable with prescaler 128
  ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  // enable with prescaler 64
//  ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0);

  // AV ref
  ADMUX = (1 << REFS0);


  return 1;
}

void sensor_setTrigger(uint8_t channel, uint16_t threshold, uint8_t triggerOnLow){
  sensor_threshold = threshold;
  sensor_triggerOnLow = triggerOnLow;
  sensor_channel = channel;
  sensor_resetTrigger();
}

void sensor_resetTrigger(void){
  sensor_triggeredValue = 0;

  // read twice, just to make sure it is clean
  sensor_read(sensor_channel);
  sensor_read(sensor_channel);
  sensor_freeRunning = 1;
  sensor_selectChannel(sensor_channel);
  sensor_setIRQ(1);
  SENSOR_ADC_START();
}

void sensor_abortTrigger(void){
  sensor_setIRQ(0);
  while (ADCSRA & (1<<ADSC) );
  sensor_freeRunning = 0;
}

uint16_t sensor_read(uint8_t channel){
  if(sensor_freeRunning){
    return 0;
  }
  sensor_selectChannel(channel);
  SENSOR_ADC_START();
  while (ADCSRA & (1<<ADSC) );
  return ADCW;
}

uint8_t sensor_wasTriggered(void){
  return (!sensor_freeRunning);
}

uint16_t sensor_getTriggeredValue(void){
  return sensor_triggeredValue;
}
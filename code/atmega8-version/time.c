#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "pins.h"
#include "time.h"
#include "ui.h"

volatile uint32_t time_currentTime;


uint8_t time_debounceCounter = 0;

ISR(TIMER0_COMPA_vect){
  time_currentTime++;
  if(time_debounceCounter) {
    time_debounceCounter--;
  } else {
    UI_DEBOUNCE_IRQ();
    time_debounceCounter = TIME_DEBOUNCE_INTERVALL;
  }
}




uint8_t time_setup(void){
  time_currentTime = 0;

  // set timer 0 to CTC mode, prescaler 64 top at 250
  TCCR0A = (1 << WGM01);
  TCCR0B = (1 << CS01) | ( 1 << CS00);
  TIMSK0 = (1 << OCIE0A);
  OCR0A = 250;

  return 1;
}

uint32_t time_getMS(void){
  return time_currentTime;
}

uint32_t time_getSec(void){
  return (time_currentTime / 1000);
}

uint32_t time_getDifference(uint32_t start, uint32_t end){
  uint32_t diff = 0;
  if(end >= start){
    return (end - start);
  }
  diff  = 0xFFFFFFFF - start;
  diff += end;
  return diff;
}

void time_delayMS(uint32_t delay){
  while(delay){
    _delay_ms(1);
    delay--;
  }
}

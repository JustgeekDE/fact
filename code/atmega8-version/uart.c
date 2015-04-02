#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "uart.h"

#define UART_ENABLE_TX_INT()  {UCSR0B |=  (1<<UDRIE0);}
#define UART_DISABLE_TX_INT() {UCSR0B &= ~(1<<UDRIE0);}

typedef struct {
	uint8_t writePointer;
	uint8_t readPointer;
	uint8_t size;
	char data[UART_BUFFER_SIZE];
} uart_buffer;


char bufferGetData(volatile uart_buffer* buffer) {
	char data = 0;
	if(buffer->size) {
		buffer->size--;
		data = buffer->data[buffer->readPointer];
		buffer->readPointer++;
		while (buffer->readPointer >= UART_BUFFER_SIZE) {
			buffer->readPointer -= UART_BUFFER_SIZE;
		}
	}
	return data;
}

char bufferPutData(volatile uart_buffer* buffer, char data) {
	if(buffer->size < UART_BUFFER_SIZE) {
		buffer->size++;
		buffer->data[buffer->writePointer] = data;
		buffer->writePointer++;
		while (buffer->writePointer >= UART_BUFFER_SIZE) {
			buffer->writePointer -= UART_BUFFER_SIZE;
		}
		return 1;
	}
	return 0;
}

char bufferHasData(volatile uart_buffer* buffer) {
	return buffer->size;
}

char bufferIsFull(volatile uart_buffer* buffer) {
	return (buffer->size >= UART_BUFFER_SIZE);
}


volatile uart_buffer rxBuffer;
volatile uart_buffer txBuffer;

uint8_t uart_setup(void) {
  UART_TX_DDR |= ( 1 << UART_TX_BIT);
  UART_RX_DDR &=~( 1 << UART_RX_BIT);

  UART_RX_PORT &=~( 1 << UART_RX_BIT); // Disable pullup

  // set baudrate
  #include <util/setbaud.h>
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;

  UCSR0B |= (1<<TXEN0) | (1<<RXEN0) | (1<<RXCIE0); // Enable Transmission, reciever, reciever interrupt
  UCSR0C |= (1 << UCSZ01)|(1 << UCSZ00); // Asynchron 8N1

  rxBuffer.writePointer = 0;
  rxBuffer.readPointer = 0;
  rxBuffer.size = 0;

  txBuffer.writePointer = 0;
  txBuffer.readPointer = 0;
  txBuffer.size = 0;

  return 1;
}


uint8_t uart_putcBlocking(unsigned char c){
  while (!(UCSR0A & (1<<UDRE0))){
    // Busy wait
  }
  UDR0 = c;  // set char to send
  return 1;
}

void uart_putsBlocking(char *s){
  while (*s) {
    uart_putcBlocking(*s);
    s++;
  }
}

void uart_putsBlocking_P(char *s){
  char c;
  while((c = pgm_read_byte(s++)) != '\0' ) {
    uart_putcBlocking(c);
  }
}

uint8_t uart_putcNonBlocking(unsigned char c){
  while (bufferIsFull(&txBuffer)){
    // Busy wait
  }
  bufferPutData(&txBuffer, c);
  UART_ENABLE_TX_INT();
  return 1;
}

void uart_putsNonBlocking(char *s){
  while (*s) {
    uart_putcNonBlocking(*s);
    s++;
  }
}

void uart_putsNonBlocking_P(char *s){
  char c;
  while((c = pgm_read_byte(s++)) != '\0' ) {
    uart_putcNonBlocking(c);
  }
}

uint8_t uart_putc(unsigned char c){
  return uart_putcBlocking(c);
}

void uart_puts(char *s){
  return uart_putsBlocking(s);
}

void uart_puts_P(char *s){
  return uart_putsBlocking_P(s);
}

char uart_hasData() {
	return bufferHasData(&rxBuffer);
}

char uart_getChar() {
	return bufferGetData(&rxBuffer);
}


// Reciever interrupt
ISR(USART_RX_vect) {
  unsigned char data;
  data = UDR0;
  bufferPutData(&rxBuffer, data);
}


//sender Interrupt
ISR(USART_UDRE_vect) {
  unsigned char data;
  data = bufferGetData(&txBuffer);
  UDR0 = data;  // set char to send
  if (!bufferHasData(&txBuffer)){
	  UART_DISABLE_TX_INT();
  }

}


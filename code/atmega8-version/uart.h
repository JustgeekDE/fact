#ifndef __UART_H
#define __UART_H
#include <stdint.h>
#include <avr/io.h>

#define BAUD 19200

#define UART_TX_PORT PORTD
#define UART_TX_PIN  PIND
#define UART_TX_DDR  DDRD
#define UART_TX_BIT  1

#define UART_RX_PORT PORTD
#define UART_RX_PIN  PIND
#define UART_RX_DDR  DDRD
#define UART_RX_BIT  0

#define UART_BUFFER_SIZE 64

extern uint8_t uart_setup(void);
extern uint8_t uart_putc(unsigned char c);
extern void uart_puts (char *s);
extern void uart_puts_P (char *s);

extern char uart_getChar(void);
extern char uart_hasData(void);




#endif
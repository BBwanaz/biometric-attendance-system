/*
 * uart.c
 *
 * Asynchronous UART example tested on ATMega328P (16 MHz)
 *
 * Toolchain: avr-gcc (4.3.3)
 * Editor: Eclipse Kepler (4)
 * Usage:
 * 		Perform all settings in uart.h and enable by calling initUART(void)
 * 		Compile:
 * 				make all
 *
 * Functions:
 * 		- First call initUART() to set up Baud rate and frame format
 *		- initUART() calls macros TX_START() and RX_START() automatically
 *		- To enable interrupts on reception, call RX_INTEN() macros
 *		- Call functions getByte() and putByte(char) for character I/O
 *		- Call functions writeString(char*) and readString() for string I/O
 *
 *  Created on: 21-Jan-2014
 *      Author: Shrikant Giridhar
 */

#include "usart.h"

void initUART(void)
{
	// Not necessary; initialize anyway
	DDRD |= _BV(PD1);
	DDRD &= ~_BV(PD0);

	// Set baud rate; lower byte and top nibble
	UBRR0H = ((_UBRR) & 0xF00);
	UBRR0L = (uint8_t) ((_UBRR) & 0xFF);

	TX_START();
	RX_START();

	// Set frame format = 8-N-1
	UCSR0C = (_DATA << UCSZ00);
}

uint8_t getByte(void)
{
	// Check to see if something was received
	while (!(UCSR0A & _BV(RXC0)));
	return (uint8_t) UDR0;
}

void putByte(unsigned char data)
{
	// Stay here until data buffer is empty
	while (!(UCSR0A & _BV(UDRE0)));
	UDR0 = (unsigned char) data;
}

void writeString(char *str)
{
	while (*str != '\0')
	{
		putByte(*str);
		++str;
	}
}

char * readString(void)
{
	static char rxstr[RX_BUFF];
	static char* temp;
	temp = rxstr;

	while((*temp = getByte()) != '\r')
	{
		++temp;
	}
	*temp=0;

	return rxstr;
}

int main(void)
{
	char *ps;
	initUART();
	writeString("Hello,World!\r\n");

	while(1)
	{
		writeString("Type something: ");
		ps=readString();
		writeString("\r\nYou typed: ");
		writeString(ps);
		putByte('\r');
		putByte('\n');
	}
	return 0;
}

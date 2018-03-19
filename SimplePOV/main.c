/*
 * SimplePOV.c
 *
 * Created: 1/11/2018 4:51:12 AM
 * Author : Donghyeon Lee
 * Version 2.1
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/fuse.h>
#define F_CPU 16000000UL // CPU Clock Freq.

#define TVAL 1000 // Time interval setting

#include "font.h" // Font data

const char str[] __attribute__((progmem)) = "Hello, SCSC!"; // String to be displayed (Stored in code memory)

const unsigned short DLEN = FONTWIDTH * (sizeof(str) - 1); // Display data length(Font width*String length)

// Fuse bit setting
FUSES =
{
	.low = FUSE_SUT0 & FUSE_SUT1, // Ext. Xtal Osc. > 8MHz, 63mS SUT
	.high = (FUSE_BOOTSZ0 & FUSE_BOOTSZ1 & FUSE_EESAVE & FUSE_SPIEN), // No bootldr, spi, eeprom preserved
	.extended = FUSE_BODLEVEL1, // BOD 2.7V
};

unsigned short pos=0; // Data position index

// Timer ISR
ISR(TIMER1_COMPA_vect)
{
	// LED control.
	PORTC &= ~0b111111;
	PORTC |= ~font[(int)str[pos / 6]][pos % 6] & 0b111111; // 6LSB
	PORTB &= ~0b11;
	PORTB |= ( ~font[(int)str[pos / 6]][pos % 6] >> 6) & 0b11; // 2MSB
	pos++; // Increase data position index
	if(pos >= DLEN) // end of display data
	{
		pos=0;
		TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); // stop timer
		TIMSK1 &= ~(1 << OCIE1A); // Disable timer interrupt
	}
	TIFR1 &= ~(1<<OCF1A); // clear flag

}

// EXTINT 0 (Photo interrupter) ISR
ISR(INT0_vect)
{
	TIMSK1 |= (1 << OCIE1A); // Enable timer interrupt
	TCNT1 = 0; // Reset Counter
	TCCR1B |= (1 << CS11); // Timer prescaler to CLK/8
	EIFR &= ~(1 << INTF0); // Clear flag
}

// Main program
int main()
{
	// Initialization
	DDRD&= ~(1 << PORTD2); // Set INT0 as input
	PORTC |= 0x3F; // turn LEDs off
	PORTB |= 0x03;
	DDRC |= 0x3F; // set led ports to output
	DDRB |= 0x03;
	EICRA = (1 << ISC01); // EXTINT 0 on falling edge.
	EIMSK = (1 << INT0); // enable extint0
	TCCR1A = 0; // No comp. output
	TCCR1B = (1 << WGM12); // CTC mode
	OCR1A = TVAL; // Comparator value
	TCNT1 = 0; // reset counter
	sei(); // Enable global interrupt
	
	// Main Loop
	while(1)
	{
		; // Do nothing. Tasks are triggered by interrupts.
	}
}



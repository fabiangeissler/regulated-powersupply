/*
 * display.c
 *
 *  Created on: Aug 21, 2016
 *      Author: fabian
 */

#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include "stdlib.h"

#include "segment_display.h"

#define SEGA	(1 << 6)
#define SEGB	(1 << 7)
#define SEGC	(1 << 0)
#define SEGD	(1 << 3)
#define SEGE	(1 << 2)
#define SEGF	(1 << 5)
#define SEGG	(1 << 4)
#define SEGPT	(1 << 1)

// Character to segment mapping. Segment-bit order: c.pt.e.d.g.f.a.b
uint8_t _char_map[10] = {
		SEGA | SEGB | SEGC | SEGD | SEGE | SEGF,		// 0
		SEGB | SEGC,									// 1
		SEGA | SEGB | SEGG | SEGE | SEGD,				// 2
		SEGA | SEGB | SEGG | SEGC | SEGD,				// 3
		SEGB | SEGC | SEGG | SEGF,						// 4
		SEGA | SEGF | SEGG | SEGC | SEGD,				// 5
		SEGA | SEGG | SEGC | SEGD | SEGE | SEGF,		// 6
		SEGA | SEGB | SEGC,								// 7
		SEGA | SEGB | SEGC | SEGD | SEGE | SEGF | SEGG,	// 8
		SEGA | SEGB | SEGC | SEGD | SEGF | SEGG,		// 9
};

#define DIGIT_DDR	DDRD
#define DIGIT_PORT	PORTD
#define SEG01_DDR	DDRB
#define SEG01_PORT	PORTB
#define SEG27_DDR	DDRC
#define SEG27_PORT	PORTC

uint8_t _display_string[8];

void initIO()
{
	DIGIT_DDR = 0xFF;
	DIGIT_PORT = 0x01;

	SEG01_DDR |= 0x07;
	SEG01_PORT &= ~0x07;

	SEG27_DDR |= 0x3E;
	SEG27_PORT &= ~0x3E;
}

void initTimer()
{
	TCCR0 = (0 << CS02) | (1 << CS01) | (1 << CS00);
	TIMSK = (1 << TOIE0);
}

// Init display timer (TIMER0), inputs and outputs
void segmentDisplayInit(void)
{
	displayValues(-1.234,567.8);

	PORTC |= 0x8;

	initIO();
	initTimer();
}

void floatToStr(float f, uint8_t max_dig, uint8_t * str)
{
	if((f >= 10000) || (f <= -1000)) // Overflow
	{
		str[0] = 0;
		str[1] = 0;
		str[2] = SEGG | SEGC | SEGD | SEGE | SEGPT;
		str[3] = SEGC | SEGD | SEGE | SEGPT;
	}
	else
	{
		char tmp[6];
		dtostrf(f, 5, max_dig, tmp);

		for(int i = 0, s = 0; (i < 6) && (s < 4); ++i)
		{
			if((s >= 1) && (tmp[i] == '.'))
				str[s - 1] |= SEGPT;
			else if(tmp[i] == '-')
				str[s++] = SEGG;
			else if((tmp[i] >= '0') && (tmp[i] <= '9'))
				str[s++] = _char_map[tmp[i] - '0'];
			else
				str[s++] = 0;
		}
	}
}

void displayValues(float val0, float val1)
{
	floatToStr(val0, 2, _display_string);
	floatToStr(val1, 1, _display_string + 4);
}

void outputSegments(uint8_t seg, uint8_t digit)
{
	uint8_t seg01 = (SEG01_PORT & ~0x07) | (seg & 0x07);
	uint8_t seg27 = (SEG27_PORT & ~0x3E) | ((seg & 0xF8) >> 2);
	uint8_t dig = (0x80 >> digit);

	DIGIT_PORT = dig;
	SEG01_PORT = seg01;
	SEG27_PORT = seg27;
}

// Timer interrupt for digit multiplexing
ISR(TIMER0_OVF_vect)
{
	static uint8_t digit = 0;

	//SEG27_PORT |= 0x02;
	outputSegments(_display_string[digit], digit);

	digit = (digit + 1) & 0b111;
}

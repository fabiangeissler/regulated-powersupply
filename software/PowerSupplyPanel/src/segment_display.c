/*
 * display.c
 *
 *  Created on: Aug 21, 2016
 *      Author: fabian
 */

#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>

#define INTLEN(v)    (((v) >= 1000) ? 3 : ((v) >= 100) ? 2 : ((v) >= 10) ? 1 : 0))

#define SEGA	(1 << 6)
#define SEGB	(1 << 7)
#define SEGC	(1 << 0)
#define SEGD	(1 << 3)
#define SEGE	(1 << 2)
#define SEGF	(1 << 5)
#define SEGG	(1 << 4)
#define SEGPT	(1 << 1)

// Character to segment mapping. Segment-bit order: c.pt.e.d.g.f.a.b
uint8_t mCharacterMap[10] = {
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

void initIO()
{
	DIGIT_DDR = 0xFF;
	DIGIT_PORT = 0x00;

	SEG01_DDR |= 0x03;
	SEG01_PORT &= ~0x03;

	SEG27_DDR |= 0x3F;
	SEG27_PORT &= ~0x3F;
}

void initTimer()
{
	sei();

	TCCR0 = (0 << CS02) | (1 << CS01) | (1 << CS00);
	TIMSK = (1 << TOIE0);
}

// Init display timer (TIMER0), inputs and outputs
void segmentDisplayInit(void)
{
	initIO();
	initTimer();
}

void setSegments(uint8_t seg)
{
	SEG01_PORT |= (seg & 0x03);
	SEG27_PORT |= ((seg << 2) & 0x3F);
}

void setDigit(uint8_t digit)
{
	DIGIT_PORT = (1 << digit);
}

void floatToStr(float f, uint8_t * str)
{
	if(f >= 10000) // Overflow
	{
		str[0] = 0;
		str[1] = 0;
		str[2] = SEGG | SEGC | SEGD | SEGE | SEGPT;
		str[3] = SEGC | SEGD | SEGE | SEGPT;
	}

	int l = INTLEN((int)f);


}

void createDisplayString(float val0, float val1, uint8_t * str)
{

}

// Timer interrupt for digit multiplexing
ISR(TIMER0_OVF_vect)
{
	static uint8_t digit = 0;
}

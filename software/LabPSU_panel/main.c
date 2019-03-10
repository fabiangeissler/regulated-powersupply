/*
 * main.c
 *
 *  Created on: Aug 21, 2016
 *      Author: fabian
 */


#include <avr/io.h>
#include <stdint.h>
#include "avr/interrupt.h"
#include "stdbool.h"
#include "segment_display.h"

// Enter the value, measured, when the display shows the named value, when all other calibration constants are dafult
#define CALIBRATION_1V_OFFSET 1.053//1.129//1.053
#define CALIBRATION_14V_OFFSET 14.41//13.97//14.41
#define CAL_VDM	((CALIBRATION_14V_OFFSET - CALIBRATION_1V_OFFSET) / 13.0)
#define CAL_0V	(CALIBRATION_1V_OFFSET - CAL_VDM)

#define CALIBRATION_0A_OFFSET -5.4//-4.79//-5.4
#define CALIBRATION_500mA_OFFSET 508.5//487//508.5
#define CAL_CDM	((CALIBRATION_500mA_OFFSET - CALIBRATION_0A_OFFSET) / 500.0)

#define LONG_AVERAGE	10
#define LONG_PART_V		0.05
#define LONG_PART_C		0.1
#define ADC_AVERAGES	400

enum {
	ADC_CURRENT = 0x00,			// ADC0
	ADC_VOLTAGE_POS = 0x01,		// ADC6
	ADC_VOLTAGE_NEG = 0x02,		// ADC7
	ADC_INVERT_VOLTAGE = 0x04
};

// ADC flags for channel selection
volatile uint8_t adc_flags = ADC_CURRENT;

// adc_mux are the multiplexer bits based on the adc selection in adc_flags
uint8_t adc_mux[3] = { 0b0000 /*ADC0*/, 0b0110 /*ADC6*/, 0b0111 /*ADC7*/ };

// averaging memory
volatile int32_t adc_current, adc_current_buffer;
volatile int32_t adc_voltage, adc_voltage_buffer;
volatile uint16_t adc_count = 0;
volatile bool adc_refresh = false;

void beginAdcOperation()
{
	ADMUX = (1 << REFS1) | (1 << REFS0) | adc_mux[adc_flags & 0b11];
	// Prescaler should be 80 for 200kHz sampling clock. set to 64 or 128
	ADCSRA = (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	// enable and start conversion
	ADCSRA |= (1 << ADEN) | (1 << ADSC);
}

void setAdcAdmux(uint8_t mux)
{
	ADMUX &= 0xF0;
	ADMUX |= mux;
}

ISR(ADC_vect)
{
	uint8_t flags = adc_flags;
	uint8_t converter = flags & 0b11;

	if(converter == 0)
	{
		// store result
		adc_current += ADC;

		// select next channel
		if(flags & ADC_INVERT_VOLTAGE)
			converter = ADC_VOLTAGE_NEG;
		else
			converter = ADC_VOLTAGE_POS;
	}
	else
	{
		int16_t res = ADC;

		if(flags & ADC_INVERT_VOLTAGE)
			res = -res;

		// store result
		adc_voltage += res;

		// switch inverting mode
		if(res == 0)
			flags ^= ADC_INVERT_VOLTAGE;

		// new offset
		++adc_count;

		if(adc_count >= ADC_AVERAGES)
		{
			adc_count = 0;

			adc_voltage_buffer = adc_voltage;
			adc_voltage = 0;

			adc_current_buffer = adc_current;
			adc_current = 0;

			// set display refresh
			adc_refresh = true;
		}

		// select next channel
		converter = ADC_CURRENT;
	}

	// update flags
	adc_flags = (flags & 0xFC) | converter;

	// select next channel
	setAdcAdmux(adc_mux[converter]);
	// start conversion
	ADCSRA |= (1 << ADSC);
}

int main(void)
{
	sei();

	segmentDisplayInit();

	beginAdcOperation();

	uint16_t count = 0;

	float long_voltage[LONG_AVERAGE] = {0};
	float long_current[LONG_AVERAGE] = {0};

	while(1)
	{
		if(adc_refresh)
		{
			adc_refresh = false;

			float voltage = CAL_0V + CAL_VDM * 15.0 * 2.56 / (1.36363636 * 1024.0 * ADC_AVERAGES) * (float)adc_voltage_buffer;
			float current = CALIBRATION_0A_OFFSET + CAL_CDM * 1000 * 2.56 / (1024.0 * ADC_AVERAGES) * (float)adc_current_buffer;

			long_voltage[count] = voltage;
			long_current[count] = current;
			++count;

			if(count == LONG_AVERAGE)
				count = 0;

			float sum_v = 0;
			float sum_c = 0;

			for(int i = 0; i < LONG_AVERAGE; ++i)
			{
				sum_v += long_voltage[i];
				sum_c += long_current[i];
			}

			sum_v /= LONG_AVERAGE;
			sum_c /= LONG_AVERAGE;

			displayValues(voltage * (1 - LONG_PART_V) + sum_v * LONG_PART_V, current * (1 - LONG_PART_C) + sum_c * LONG_PART_C);
		}
	}
}


///*
// * main.c
// *
// *  Created on: Aug 21, 2016
// *      Author: fabian
// */
//
//
//#include <avr/io.h>
//#include <stdint.h>
//#include "avr/interrupt.h"
//#include "stdbool.h"
//#include "segment_display.h"
//
//#define ADC_AVERAGES	200
//
//enum {
//	ADC_CURRENT = 0x00,			// ADC0
//	ADC_VOLTAGE_POS = 0x01,		// ADC6
//	ADC_VOLTAGE_NEG = 0x02,		// ADC7
//	ADC_INVERT_VOLTAGE = 0x04
//};
//
//// ADC flags for channel selection
//volatile uint8_t adc_flags = ADC_CURRENT;
//
//// adc_mux are the multiplexer bits based on the adc selection in adc_flags
//uint8_t adc_mux[3] = { 0b0000 /*ADC0*/, 0b0110 /*ADC6*/, 0b0111 /*ADC7*/ };
//
//// averaging memory
//int16_t adc_current_mem[ADC_AVERAGES];
//int16_t adc_voltage_mem[ADC_AVERAGES];
//volatile uint8_t adc_mem_offset = 0;
//volatile bool adc_refresh = false;
//
//void beginAdcOperation()
//{
//	ADMUX = (1 << REFS1) | (1 << REFS0) | adc_mux[adc_flags & 0b11];
//	// Prescaler should be 80 for 200kHz sampling clock. set to 64 or 128
//	ADCSRA = (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0);
//	// enable and start conversion
//	ADCSRA |= (1 << ADEN) | (1 << ADSC);
//}
//
//void setAdcAdmux(uint8_t mux)
//{
//	ADMUX &= 0xF0;
//	ADMUX |= mux;
//}
//
//ISR(ADC_vect)
//{
//	uint8_t flags = adc_flags;
//	uint8_t converter = flags & 0b11;
//
//	if(converter == 0)
//	{
//		// store result
//		adc_current_mem[adc_mem_offset] = ADC;
//
//		// select next channel
//		if(flags & ADC_INVERT_VOLTAGE)
//			converter = ADC_VOLTAGE_NEG;
//		else
//			converter = ADC_VOLTAGE_POS;
//	}
//	else
//	{
//		int16_t res = ADC;
//
//		if(flags & ADC_INVERT_VOLTAGE)
//			res = -res;
//
//		// store result
//		adc_voltage_mem[adc_mem_offset] = res;
//
//		// switch inverting mode
//		if(res == 0)
//			flags ^= ADC_INVERT_VOLTAGE;
//
//		// set display refresh
//		adc_refresh = true;
//
//		// new offset
//		++adc_mem_offset;
//
//		if(adc_mem_offset >= ADC_AVERAGES)
//			adc_mem_offset = 0;
//
//		// select next channel
//		converter = ADC_CURRENT;
//	}
//
//	// update flags
//	adc_flags = (flags & 0xFC) | converter;
//
//	// select next channel
//	setAdcAdmux(adc_mux[converter]);
//	// start conversion
//	ADCSRA |= (1 << ADSC);
//}
//
//int main_movav(void)
//{
//	sei();
//
//	segmentDisplayInit();
//
//	beginAdcOperation();
//
//	while(1)
//	{
//		if(adc_refresh)
//		{
//			adc_refresh = false;
//
//			int32_t sum = 0;
//
//			for(int i = 0; i < ADC_AVERAGES; ++i)
//				sum += adc_voltage_mem[i];
//
//			float voltage = 15.0 * 2.56 / (1.36363636 * 1024.0 * ADC_AVERAGES) * (float)sum;
//
//			displayValues(voltage, -10);
//		}
//	}
//}
//

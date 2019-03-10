/*
 * display.h
 *
 *  Created on: Aug 21, 2016
 *      Author: fabian
 */

#ifndef SEGMENT_DISPLAY_H_
#define SEGMENT_DISPLAY_H_

#include <avr/io.h>

void segmentDisplayInit(void);

void displayValues(float val0, float val1);

#endif /* SEGMENT_DISPLAY_H_ */

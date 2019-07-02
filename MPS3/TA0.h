/*
 * TA0.h
 *
 *  Created on: 22.04.2018
 *      Author: Admin
 */

#include "base.h"

#ifndef TA0_H_
#define TA0_H_

EXTERN Void TA0_Init(Void);

#define MUSTER1 0
#define MUSTER2 1
#define MUSTER3 2
#define MUSTER4 3
#define MUSTER5 4
#define MUSTER6 5

LOCAL UInt LEDarr1[] = {200, 50, 0};
LOCAL UInt LEDarr2[] = {75, 75, 0};
LOCAL UInt LEDarr3[] = {25, 25, 0};
LOCAL UInt LEDarr4[] = {50, 200, 0};
LOCAL UInt LEDarr5[] = {50, 50, 50, 200, 0};
LOCAL UInt LEDarr6[] = {50, 50, 50, 50, 50, 200, 0};

LOCAL UInt* LED_arr[] = {LEDarr1, LEDarr2, LEDarr3, LEDarr4, LEDarr5, LEDarr6};
LOCAL UInt* LED_ERROR_arr[] = {LEDarr1, LEDarr3, LEDarr4, LEDarr5, LEDarr6};

#define LEDmax 25
#define HYSTMAX 5

EXTERN Void set_blink_muster(UInt);
EXTERN Void process_error(UInt);

#endif /* TA0_H_ */

/*
 * TA0.c
 *
 *  Created on: 22.04.2018
 *      Author: Admin
 */

#include <msp430.h>
#include "..\..\..\base.h"
#include "TA0.h"
#include "event.h"

/*
 * Man soll sich geeignete Datenstrukturen überlegen,
 * die eine laufzeitefffiziente Auusführung der ISR
 * ermöglichen.
 */
LOCAL UInt* LEDptr = &LEDarr1;
LOCAL UShort LEDcnt = 25;
LOCAL UShort hyst = 0;
LOCAL UInt LEDarr1[] = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0}

GLOBAL Void set_blink_muster(UInt arg) {
/*
 * Die Funktion muss so ertweitert werden,
 * dass ein Blinkmuster selektiert wird.
 */
}

// Der Timer A0 ist bereits initialisiert
GLOBAL Void TA0_Init(Void) {
   TA0CCR0 = 0;                              // stopp Timer A
   CLRBIT(TA0CTL, TAIFG);                    // clear overflow flag
   CLRBIT(TA0CCR0, CCIFG);                   // clear CCI flag
   TA0CTL  = TASSEL__ACLK + MC__UP + ID__8;  // set up Timer A
   TA0EX0  = TAIDEX_7;                       // set up expansion register
   TA0CCR0 = 2*48;                           // set up CCR0 for 10 ms
   SETBIT(TA0CTL, TACLR);                    // clear and start Timer
   SETBIT(TA0CCTL0, CCIE);                   // enable Timer A interrupt
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt Void TA0_ISR(Void) {
   /*
    * Der Inhalt der ISR ist zu implementieren
    */
    if(--LEDcnt EQ 0) {
        LEDcnt = LEDmax;
        LEDptr += 1;
        SETBIT(P1OUT, BIT2);
    }
    if(TSTBIT(P1IN, BIT0)) {
        ++BTN1hyst;
        if(BTN1hyst EQ 5) {
            set_event(EVENT_BTN1);
            __low_power_mode_off_on_exit();
        }
    } elsif(BTN1hyst GT 0) {
        BTN1hyst--;
    }
    if(TSTBIT(P1IN, BIT1)) {
        ++BTN2hyst;
        if(BTN2hyst EQ 5) {
            set_event(EVENT_BTN2);
            __low_power_mode_off_on_exit();
        }
    } elsif(BTN1hyst GT 0) {
        BTN2hyst--;
    }
}

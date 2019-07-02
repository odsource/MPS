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
//LOCAL UInt* LED_arr_ptr = &LEDarr1;
LOCAL UInt* LED_arr_ptr = &LEDarr1;
LOCAL UShort LEDcnt = 200;
LOCAL UShort array_length = 3;
LOCAL UShort hop = sizeof(UInt) / 2;

LOCAL UShort BTN1hyst = 0;
LOCAL UShort BTN2hyst = 0;
LOCAL UShort GPIO_BTN0_hyst = 0;
LOCAL UShort GPIO_BTN1_hyst = 0;
LOCAL UShort GPIO_BTN2_hyst = 0;
LOCAL UShort GPIO_BTN3_hyst = 0;
LOCAL UInt   err_msk = 0x0000;

GLOBAL Void set_blink_muster(UInt arg) {
/*
 * Die Funktion muss so ertweitert werden,
 * dass ein Blinkmuster selektiert wird.
 */
    LED_arr_ptr = LED_arr[arg];
    if(arg LT 4) {
        array_length = 3;
    } else {
        if(arg EQ 4) {
            array_length = 5;
        }
        if(arg EQ 5) {
            array_length = 7;
        }
    }
    LEDcnt = *LED_arr_ptr;
}

GLOBAL Void process_error(UInt error_msk) {
    UInt err = 0;
    err_msk = error_msk;

    err_msk = err_msk >> 12;

    if (err_msk  EQ 1) {
        LED_arr_ptr = LED_ERROR_arr[1];
        array_length = 3;
    }
    if (err_msk >> 1 EQ 1) {
        LED_arr_ptr = LED_ERROR_arr[2];
        array_length = 3;
    }
    if (err_msk >> 2 EQ 1) {
        LED_arr_ptr = LED_ERROR_arr[3];
        array_length = 5;
    }
    if (err_msk >> 3 EQ 1) {
        LED_arr_ptr = LED_ERROR_arr[4];
        array_length = 7;
    }
    if (err_msk EQ 0x0000) {
        LED_arr_ptr = LED_ERROR_arr[0];
        array_length = 3;
    }
    LEDcnt = *LED_arr_ptr;
    SETBIT(P1OUT, BIT2);
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
   TGLBIT(P1OUT, BIT2);
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt Void TA0_ISR(Void) {
   /*
    * Der Inhalt der ISR ist zu implementieren
    */

    /*
     * Blinkmuster ändern
     */
    if(--LEDcnt EQ 0) {
        LED_arr_ptr = LED_arr_ptr + hop;
        if(*LED_arr_ptr EQ 0) {
//            if(tst_event(EVENT_BTN1)) {
//                __low_power_mode_off_on_exit();
//            }
            LED_arr_ptr = LED_arr_ptr - ((array_length - 1) * hop);
        }
        LEDcnt = *LED_arr_ptr;
        TGLBIT(P1OUT, BIT2);
    }

    /*
     * Buttonabfrage Blinkmuster ändern
     */
//    if(TSTBIT(P1IN, BIT1)) {
//        if(BTN1hyst LT HYSTMAX) {
//            ++BTN1hyst;
//            if(BTN1hyst EQ HYSTMAX) {
//                set_event(EVENT_BTN1);
//            }
//        }
//    } else if(BTN1hyst GT 0) {
//        BTN1hyst--;
//    }

    if(TSTBIT(P1IN, BIT0)) {
        if(BTN2hyst LT HYSTMAX) {
            ++BTN2hyst;
            if(BTN2hyst EQ HYSTMAX) {
                set_event(EVENT_BTN2);
                __low_power_mode_off_on_exit();
            }
        }
    } else if(BTN2hyst GT 0) {
        BTN2hyst--;
    }

    if(TSTBIT(P3IN, BIT0)) {
        if(GPIO_BTN0_hyst LT HYSTMAX) {
            ++GPIO_BTN0_hyst;
            if(GPIO_BTN0_hyst EQ HYSTMAX) {
                set_event(EVENT_BTN3);
                __low_power_mode_off_on_exit();
            }
        }
    } else if(GPIO_BTN0_hyst GT 0) {
        GPIO_BTN0_hyst--;
    }

    if(TSTBIT(P3IN, BIT1)) {
        if(GPIO_BTN1_hyst LT HYSTMAX) {
            ++GPIO_BTN1_hyst;
            if(GPIO_BTN1_hyst EQ HYSTMAX) {
                set_event(EVENT_BTN4);
                __low_power_mode_off_on_exit();
            }
        }
    } else if(GPIO_BTN1_hyst GT 0) {
        GPIO_BTN1_hyst--;
    }

    if(TSTBIT(P3IN, BIT2)) {
        if(GPIO_BTN2_hyst LT HYSTMAX) {
            ++GPIO_BTN2_hyst;
            if(GPIO_BTN2_hyst EQ HYSTMAX) {
                set_event(EVENT_BTN5);
                __low_power_mode_off_on_exit();
            }
        }
    } else if(GPIO_BTN2_hyst GT 0) {
        GPIO_BTN2_hyst--;
    }

    if(TSTBIT(P3IN, BIT3)) {
        if(GPIO_BTN3_hyst LT HYSTMAX) {
            ++GPIO_BTN3_hyst;
            if(GPIO_BTN3_hyst EQ HYSTMAX) {
                set_event(EVENT_BTN6);
                __low_power_mode_off_on_exit();
            }
        }
    } else if(GPIO_BTN3_hyst GT 0) {
        GPIO_BTN3_hyst--;
    }

}

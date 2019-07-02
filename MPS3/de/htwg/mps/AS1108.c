/*
 * SPI_AS1108.c
 *
 *  Created on: 26.12.2018
 *      Author: Admin
 */


#include <msp430.h>
#include "..\..\..\base.h"
#include "event.h"
#include "AS1108.h"
#include "..\..\..\UART.h"

// Basis des Zahlensystems
// Einstellung zwischen 2 und 10 soll möglich sein
#define BASE 10

// es sind geeignete Datenstrukturen für den Datenaustausch
// zwischen den Handlern festzulegen.
LOCAL Int button_mask = 0x00;
EXTERN Int digit_arr[] = {0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00};
LOCAL Int* digit_ptr = &digit_arr[1];
LOCAL Int overflow = 0;
LOCAL Int digit_ctr = 0;
LOCAL Char *ptr = digit_arr;

typedef Void (* VoidFunc)(Void);

// Funktionsprototypen
LOCAL Void State0(Void);
LOCAL Void State1(Void);
LOCAL Void State2(Void);

LOCAL VoidFunc state = State0;
LOCAL UInt i;
LOCAL
Int first = 0;
Char first_char = '0';

LOCAL Void AS1108_Write(UChar adr, UChar arg) {
   Char ch = UCA1RXBUF;   // dummy read, UCRXIFG := 0, UCOE := 0
   CLRBIT(P2OUT,  BIT3);  // Select aktivieren
   UCA1TXBUF = adr;       // Adresse ausgeben
   while (TSTBIT(UCA1IFG, UCRXIFG) EQ 0);
   ch = UCA1RXBUF;        // dummy read
   UCA1TXBUF = arg;       // Datum ausgeben
   while (TSTBIT(UCA1IFG, UCRXIFG) EQ 0);
   ch = UCA1RXBUF;        // dummy read
   SETBIT(P2OUT,  BIT3);  // Select deaktivieren
}


GLOBAL Void SPI_Init(Void) {
   // set up Pin 3 at Port 2 => CS output, High
   SETBIT(P2OUT,  BIT3);
   SETBIT(P2DIR,  BIT3);
   CLRBIT(P2SEL0, BIT3);
   CLRBIT(P2SEL1, BIT3);
   CLRBIT(P2REN,  BIT3);

   // set up Pins 4, 5 and 6 at Port 2
   CLRBIT(P2SEL0, BIT4 + BIT5 + BIT6);
   SETBIT(P2SEL1, BIT4 + BIT5 + BIT6);

   // set up Universal Serial Communication Interface A
   SETBIT(UCA1CTLW0, UCSWRST);        // UCA1 software reset

   // in Übereinstimung mit dem SPI-Timing-Diagramm von AS1108
   UCA1CTLW0_H = 0b10101001;          // clock phase: rising edge, ...
                                      // ... clock polarity: inactive low ...
                                      // ... MSB first, 8-bit data, SPI master mode, ...
                                      // ... 3-pin SPI, synchronus mode

   CLRBIT(UCA1CTLW0, UCSSEL1);        // select ACLK ...
   SETBIT(UCA1CTLW0, UCSSEL0);        // .. as source clock
   UCA1BRW = 0;                       // prescaler

   CLRBIT(UCA1CTLW0, UCSWRST);        // release the UCA0 for operation
}


// der Treiberbaustein AS1108 ist hier über die SPI-Schnittstelle zu initialisieren
GLOBAL Void AS1108_Init(Void) {
    // Initialize shutdown register
    AS1108_Write(0x0C, 0x81);
    // Initialize decode-mode
    AS1108_Write(0x09, 0xFF);
    // Initialize scan-limit
    AS1108_Write(0x0B, 0x03);
    // Initialize Intensity control register
    AS1108_Write(0x0A, 0x0F);

    // Set the display to all zero
    AS1108_Write(0x01, 0x00);
    AS1108_Write(0x02, 0x00);
    AS1108_Write(0x03, 0x00);
    AS1108_Write(0x04, 0x00);
}

// ----------------------------------------------------------------------------

// der Button-Handler beinhaltet keine Zustandsmaschiene
GLOBAL Void Button_Handler(Void) {
    if (tst_event(EVENT_BTN3 + EVENT_BTN4 + EVENT_BTN5 + EVENT_BTN6) NE 0x00) {
        CLRBIT(button_mask, 0xFF);
        SETBIT(button_mask, tst_event(EVENT_BTN3 + EVENT_BTN4 + EVENT_BTN5 + EVENT_BTN6));
        clr_event(button_mask);
        button_mask = button_mask >> 3;
        set_event(EVENT_DIGI);
    }
//    if (tst_event(EVENT_BTN3)) {
//        SETBIT(button_mask, EVENT_BTN3);
//        clr_event(EVENT_BTN3);
//    }
//
//    if (tst_event(EVENT_BTN4)) {
//        SETBIT(button_mask, EVENT_BTN4);
//        clr_event(EVENT_BTN4);
//    }
//
//    if (tst_event(EVENT_BTN5)) {
//        SETBIT(button_mask, EVENT_BTN5);
//        clr_event(EVENT_BTN5);
//    }
//
//    if (tst_event(EVENT_BTN6)) {
//        SETBIT(button_mask, EVENT_BTN6);
//        clr_event(EVENT_BTN6);
//    }

}

// ----------------------------------------------------------------------------

// der Number-Handler beinhaltet keine Zustandsmaschiene
GLOBAL Void Number_Handler(Void) {
    LOCAL UShort led;
    if(tst_event(EVENT_DIGI)) {
        led = TSTBIT(P2OUT, BIT7);
        if(led EQ 0x80) {
//            if(digit_ctr EQ 0) {
//                digit_ptr = &digit_arr[3];
//            }
            if(overflow GT 0) {
                overflow -= 1;
                if(*digit_ptr EQ 0) {
                    overflow += 1;
                }
                *digit_ptr = (*digit_ptr - 1) % BASE;
            }
            if(button_mask BAND 0x01) {
                if(*digit_ptr EQ 0) {
                    overflow += 1;
                }
                *digit_ptr = (*digit_ptr - 1) % BASE;
            }
            if(*digit_ptr LT 0) {
                *digit_ptr = 9;
            }
        } else if(led EQ 0x00){
            if(overflow GT 0) {
                overflow -= 1;
                if(*digit_ptr EQ 9) {
                    overflow += 1;
                }
                *digit_ptr = (*digit_ptr + 1) % BASE;
            }
            if(button_mask BAND 0x1) {
                if(*digit_ptr EQ 9) {
                    overflow += 1;
                }
                *digit_ptr = (*digit_ptr + 1) % BASE;
            }

        }
        digit_ctr += 1;
        if(digit_ctr EQ 4) {
            overflow = 0;
            digit_ctr = 0;
            clr_event(EVENT_DIGI);
            set_event(EVENT_7LED);
            //set_event(EVENT_TXD); // Aufgabe 3
        }
        digit_ptr += sizeof(UShort);
        button_mask = button_mask >> 1;
    }
}

GLOBAL Void UCA0_display(const Char * str) {
    ptr = str;
    set_event(EVENT_7LED);
    state = State0;
    button_mask = 0x0F;
    digit_arr[7] = *ptr - '0';
    ptr++;
    digit_arr[5] = *ptr - '0';
    ptr++;
    digit_arr[3] = *ptr - '0';
    ptr++;
    digit_arr[1] = *ptr - '0';
}

// ----------------------------------------------------------------------------

// der AS1108_Hander beinhaltet eine Zustandsmaschine
GLOBAL Void AS1108_Handler(Void) {
    (*state)();
}

//    Char ch = UCA1RXBUF;   // dummy read, UCRXIFG := 0, UCOE := 0
//    CLRBIT(P2OUT,  BIT3);  // Select aktivieren
//    UCA1TXBUF = adr;       // Adresse ausgeben
//    while (TSTBIT(UCA1IFG, UCRXIFG) EQ 0);
//    ch = UCA1RXBUF;        // dummy read
//    UCA1TXBUF = arg;       // Datum ausgeben
//    while (TSTBIT(UCA1IFG, UCRXIFG) EQ 0);
//    ch = UCA1RXBUF;        // dummy read
//    SETBIT(P2OUT,  BIT3);  // Select deaktivieren

LOCAL Void State0(Void) {
    if(tst_event(EVENT_7LED)) {
        state = State1;
        set_event(EVENT_STATE);
        clr_event(EVENT_7LED);
        digit_ptr = &digit_arr[0];
        i = 1;
    }
}

LOCAL Void State1(Void) {
    CLRBIT(P2OUT, BIT3);    // Select activated
    UCA1TXBUF = *digit_ptr;
    digit_ptr += sizeof(UShort) / 2;
    state = State2;
    if(i % 2 EQ 0) {
        SETBIT(P2OUT, BIT3); // Select deactivated
    }
    if(i EQ 9) {
        i = 0;
        clr_event(EVENT_STATE);
        set_event(EVENT_TXD);
        state = State0;
        digit_ptr = &digit_arr[1];

    }
}

LOCAL Void State2(Void) {
    if(TSTBIT(UCA1IFG, UCRXIFG) NE 0) {
        i += 1;

        state = State1;
    }
}

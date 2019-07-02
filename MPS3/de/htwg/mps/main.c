#include <msp430.h> 
#include "..\..\..\base.h"
#include "..\..\..\TA0.h"
#include "..\..\..\event.h"
#include "..\..\..\AS1108.h"
#include "..\..\..\UART.h"
#include <stdlib.h>

/**
 * main.c
 */

LOCAL Void CS_Init(Void);
LOCAL Void Port_Init(Void);
LOCAL UInt  error_mask = 0x0000;

GLOBAL Void main(Void) {
   Int cnt = 0;
   // stop watchdog timer
   WDTCTL = WDTPW + WDTHOLD;

   CS_Init();     // set up Clock System
   Port_Init();   // set up LED ports
   SPI_Init();
   TA0_Init();    // set up BTN Ports and Timer A0

   UCA0_Init();
   AS1108_Init();

   while(TRUE) {
      wait_for_event();

      if (tst_event(ALL_ERRORS) GT 0) {
          SETBIT(error_mask, tst_event(ALL_ERRORS));
          clr_event(error_mask);
          process_error(error_mask);
          error_mask = 0x0000;
      }

      if (tst_event(EVENT_BTN1)) {
          clr_event(EVENT_BTN1);
          process_error(0x0000);
      }

//      if (tst_event(EVENT_BTN1)) {
//         clr_event(EVENT_BTN1);
//         if (++cnt GT MUSTER6) {
//            cnt = 0;
//         }
//         set_blink_muster(cnt);
//      }
      if (tst_event(EVENT_BTN2)) {
         clr_event(EVENT_BTN2);
         TGLBIT(P2OUT, BIT7);
      }

      if (tst_event(EVENT_RXD)) {
          clr_event(EVENT_RXD);
          UCA0_display(UCA0_buf);
      }

      if (tst_event(EVENT_TXD)) {
          clr_event(EVENT_TXD);
          UCA0_buf[0] = digit_arr[7] + '0';
          UCA0_buf[1] = digit_arr[5] + '0';
          UCA0_buf[2] = digit_arr[3] + '0';
          UCA0_buf[3] = digit_arr[1] + '0';
          UCA0_buf[4] = '\r';
          UCA0_buf[5] = '\n';
          UCA0_TXD(&UCA0_buf);
      }




      // wenn die drei Handler korrekt implementiert sind,
      // kann man ihre Reihnefolge hier beliebig ändern
      Button_Handler();
      Number_Handler();
      AS1108_Handler();

      // im Falle eines Event-Errors leuchtet die LED dauerhaft
      if (is_event_error()) {
         SETBIT(P1OUT, BIT2);
      }
   }
}

LOCAL Void CS_Init(Void) {
   CSCTL0 = CSKEY;                                       // enable clock system
   CSCTL1 = DCOFSEL_3;                                   // DCO frequency = 8,0 MHz
   CSCTL2 = SELA__XT1CLK + SELS__DCOCLK + SELM__DCOCLK;  // select clock sources
   CSCTL3 = DIVA__8      + DIVS__8      + DIVM__8;       // set frequency divider
   // XT2 disabled, XT1: HF mode,  low power, no bypass
   CSCTL4 = XT2OFF + XTS + XT1DRIVE_0;
   CSCTL0_H = 0;                                         // disable clock system
}

LOCAL Void Port_Init(Void) {
   // set up Pin 7 at Port 2 => output, LED1
   CLRBIT(P2OUT,  BIT7);
   SETBIT(P2DIR,  BIT7);
   CLRBIT(P2SEL0, BIT7);
   CLRBIT(P2SEL1, BIT7);
   CLRBIT(P2REN,  BIT7);

   // set up Pin 2 at Port 1 => output, LED2
   CLRBIT(P1OUT,  BIT2);
   SETBIT(P1DIR,  BIT2);
   CLRBIT(P1SEL0, BIT2);
   CLRBIT(P1SEL1, BIT2);
   CLRBIT(P1REN,  BIT2);

   // set up Pin 0 at Port 1 => input, BTN1
   // set up Pin 1 at Port 1 => input, BTN2
   CLRBIT(P1OUT,  BIT0 + BIT1);
   CLRBIT(P1DIR,  BIT0 + BIT1);
   CLRBIT(P1SEL0, BIT0 + BIT1);
   CLRBIT(P1SEL1, BIT0 + BIT1);
   CLRBIT(P1REN,  BIT0 + BIT1);

   // set up Pin 0 at Port 3 => input, GPIO_BTN0
   // set up Pin 1 at Port 3 => input, GPIO_BTN1
   // set up Pin 2 at Port 3 => input, GPIO_BTN2
   // set up Pin 3 at Port 3 => input, GPIO_BTN3
   CLRBIT(P3OUT,  BIT0 + BIT1 + BIT2 + BIT3);
   CLRBIT(P3DIR,  BIT0 + BIT1 + BIT2 + BIT3);
   CLRBIT(P3SEL0, BIT0 + BIT1 + BIT2 + BIT3);
   CLRBIT(P3SEL1, BIT0 + BIT1 + BIT2 + BIT3);
   CLRBIT(P3REN,  BIT0 + BIT1 + BIT2 + BIT3);
}

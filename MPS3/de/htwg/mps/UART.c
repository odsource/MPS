/*
 * UART.c
 *
 *  Created on: 14.06.2019
 *      Author: chris
 */

#include <msp430.h> /* --> msp430fr5729.h */
#include "..\..\..\base.h"
#include "..\..\..\UART.h"
#include "..\..\..\event.h"
#include "..\..\..\AS1108.h"

#define ACLK_614kHz
#define DATA_SIZE 5

typedef Void (* VoidFunc)(Void);

//LOCAL Short* digit_ptr = &digit_arr[1];
LOCAL const Char EndOfTExt = '\0';
LOCAL const Char * ptr = &EndOfTExt;
Char UCA0_buf[BUFFER_SIZE];

LOCAL UInt eventtest = 0;

GLOBAL Void UCA0_Init(Void) {

    // set up Universal Serial Communication Interface A
    SETBIT(UCA0CTLW0, UCSWRST);        // UCA0 software reset
    UCA0CTLW1 = 0x0002;                // deglitch time approximately 100 ns
    UCA0CTLW0_H = 0xC0;                // even parity, LSB first,  ...
                                       // ... 8-bit data,  One stop bit,
                                       // ... UART mode, Asynchronous mode

    SETBIT(UCA0CTLW0, UCSSEL__ACLK);   // select clock source: ACLK with 614.4 kHz
    UCA0BRW = 2;                       // set clock prescaler for 9600 baud
    UCA0MCTLW_L = 10 << 4;                   // first modulation stage
    UCA0MCTLW_H = 0xD6;                // second modulation stage
    SETBIT(UCA0MCTLW, UCOS16);         // enable 16 times oversampling

    CLRBIT(P2SEL0, BIT1 | BIT0);       // set up Port 2: Pin0 => TXD, ...
    SETBIT(P2SEL1, BIT1 | BIT0);       // ... Pin1 <= RXD
    CLRBIT(P2REN,  BIT1 | BIT0);       // without pull up
    CLRBIT(P2REN,  BIT1 | BIT0);       // without pull ups

    CLRBIT(UCA0CTLW0, UCSWRST);        // release the UCA0 for operation
    SETBIT(UCA0IE, UCRXIE);            // enable receive interrupt
}

GLOBAL Int UCA0_TXD(const Char * str) {
    if (str EQ NULL) {
        return -1;
    }
    if (*ptr EQ '\0') {
        ptr = str;
        SETBIT(UCA0IFG, UCTXIFG); // set UCTXIFG
        SETBIT(UCA0IE, UCTXIE); // enable transmit interrupt
        return 0;
    }
    return -1;
}

/*
 * The UCRXIFG interrupt flag is set each time a character is received and loaded into UCAxRXBUF.
 * An interrupt request is generated if UCRXIE and GIE are also set. UCRXIFG and UCRXIE are reset
 * by a system reset PUC signal or when UCSWRST = 1. UCRXIFG is automatically reset when UCAxRXBUF
 * is read.
 */
#pragma vector = USCI_A0_VECTOR
__interrupt Void UCA0_ISR(Void) {
   LOCAL Char ch = '\0';
   LOCAL UInt idx = 0;

   switch (UCA0IV) {
      case 0x02:  // Vector 2: Receive buffer full
          if (TSTBIT(UCA0STATW, UCRXERR)) {
              ch = UCA0RXBUF; // dummy read
              idx = 0;
              set_event(ERROR_FOP);
          } else if (TSTBIT(UCA0STATW, UCFE)) {
              ch = UCA0RXBUF;
              idx = 0;
              set_event(ERROR_FOP);
          } else if (TSTBIT(UCA0STATW, UCOE)) {
              ch = UCA0RXBUF;
              idx = 0;
              set_event(ERROR_FOP);
          } else if (TSTBIT(UCA0STATW, UCPE)) {
              ch = UCA0RXBUF;
              idx = 0;
              set_event(ERROR_FOP);
          } else if (TSTBIT(UCA0STATW, UCBRK)) {
              ch = UCA0RXBUF;
              idx = 0;
              set_event(ERROR_BRK);
          } else {
              ch = UCA0RXBUF;
              if (ch EQ '\r') {
                  if (idx EQ BUFFER_SIZE - 2) {
                      UCA0_buf[idx] = '\0';
                      idx = 0;
//                      clr_event(ALL_ERRORS);
                      set_event(EVENT_BTN1);
                      CLRBIT(UCA0IE, UCRXIE); // disable rx interrupt
                      set_event(EVENT_RXD);
                      __low_power_mode_off_on_exit();
                  } else {
                      set_event(ERROR_BUF);
                      idx = 0;
                  }
              } else if (between(0x30, ch, 0x39)) {
                  if (idx LT 4) {
                      UCA0_buf[idx] = ch;
                      if (++idx EQ DATA_SIZE) {
                          // Error case: wrong format
//                          UCA0_buf[idx] = '\0';
                          idx = 0;
//                          if(ch NE '\0') {
//                          CLRBIT(UCA0IE, UCRXIE); // disable rx interrupt
//                          set_event(EVENT_RXD);
//                          __low_power_mode_off_on_exit();
//                      }
                     }
                  } else {
                      set_event(ERROR_BUF);
                      idx = 0;
                  }
              } else {
                  set_event(ERROR_CHAR);
                  idx = 0;
              }
          }
          break;
      case 0x04: // Vector 4: Transmit buffer empty
          if (*ptr NE '\0') {
              UCA0TXBUF = *ptr++;
          } else {
              ptr = &EndOfTExt;
              CLRBIT(UCA0IE, UCTXIE); // disable tx interrupt
              SETBIT(UCA0IE, UCRXIE); // enable rx interrupt
              //set_event(EVENT_TXD);
              __low_power_mode_off_on_exit();
          }
          break;
      case 0x06: // Vector 6: Start bit received
          break;
      case 0x08: // Vector 6: Transmit complete
          break;
      default:
          break;

   }
   if (tst_event(ALL_ERRORS) GT 0) {
       __low_power_mode_off_on_exit();
   }
}

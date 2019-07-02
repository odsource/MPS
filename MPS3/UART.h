/*
 * UART.h
 *
 *  Created on: 14.06.2019
 *      Author: chris
 */

#ifndef DE_HTWG_MPS_UART_H_
#define DE_HTWG_MPS_UART_H_

#define BUFFER_SIZE 6


EXTERN Void UCA0_Init(Void);
EXTERN Int UCA0_TXD(const Char * str);

EXTERN Char UCA0_buf[BUFFER_SIZE];


#endif /* DE_HTWG_MPS_UART_H_ */

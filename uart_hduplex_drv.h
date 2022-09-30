/*****************************************************************************
* uart_hduplex_drv.h
*
* Driver source for AVR UART0 interface for connection to
* a half-duplex device
*
* This is driver is interrupt driven.
* All functionality is controlled through passing information to and
* from functions.
*
* Modified: September 2022
* Created: January 14, 2015
*
*****************************************************************************/

#include    <stdint.h>

#ifndef __UART_HDUPLEX_DRV_H__
#define __UART_HDUPLEX_DRV_H__

/****************************************************************************
  Definitions
****************************************************************************/
#if (F_CPU == 8000000UL)
#define     UART_BAUD_19200     51  // UBRR0 values for 8MHz clock and U2X0 set
#define     UART_BAUD_115200    8
#elif (F_CPU == 10000000UL)
#define     UART_BAUD_19200     64  // UBRR0 values for 10MHz clock and U2X0 set
#define     UART_BAUD_115200    10
#elif (F_CPU == 16000000UL)
#define     UART_BAUD_19200     103 // UBRR0 values for 16MHz clock and U2X0 set
#define     UART_BAUD_115200    16
#else
#warning "Device clock not defined"
#endif

#define     BAUD_19200      UART_BAUD_19200
#define     BAUD_38400      UART_BAUD_38400
#define     BAUD_115200     UART_BAUD_115200

/****************************************************************************
  Function prototypes
****************************************************************************/
void    uart_initialize(uint8_t);
uint8_t uart_rx_data(uint8_t *, uint8_t);
void    uart_tx_data(uint8_t *, uint8_t);
void    uart_tx_byte(uint8_t);
uint8_t uart_rx_byte(void);
int     uart_isbyte(void);
void    uart_flush(void);
void    uart_rx_on(void);
void    uart_rx_off(void);

#endif /* __UART_HDUPLEX_DRV_H__ */

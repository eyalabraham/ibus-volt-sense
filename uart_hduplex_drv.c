/*****************************************************************************
* uart_hduplex_drv.c
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

#include    <avr/io.h>
#include    <avr/interrupt.h>

#include    "uart_hduplex_drv.h"
#include    "util.h"

/****************************************************************************
  Definitions
****************************************************************************/
#define     UART_BUFF_LEN       32  // 32 characters in UART input buffer

/****************************************************************************
  Globals
****************************************************************************/
uint8_t     uart_buffer[UART_BUFF_LEN];
volatile    int inIndex;
volatile    int outIndex;
volatile    int bytesInBuffer;

/* ---------------------------------------------------------------------------
 * uart_initialize()
 *
 * Hard coded UART setup for now
 *
 */
void uart_initialize(uint8_t baud_rate_div)
{
    inIndex = 0;
    outIndex = 0;
    bytesInBuffer = 0;

    UCSR0A = _BV(U2X0);                 // Double baud rate (sec 19.10 p.195)
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 8 data bits, 1 stop, no parity
    UBRR0 = baud_rate_div;

    UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);   // enable Tx and Rx
}

/* ---------------------------------------------------------------------------
 * uart_rx_data()
 *
 * Attempt to read 'byteCount' data bytes from the UART input buffer.
 * Function reads as much data as possible and returns number of data bytes
 * actually read.
 *
 */
uint8_t uart_rx_data(uint8_t *data, uint8_t byteCount)
{
    uint8_t rxCount;
    uint8_t i;

    if ( bytesInBuffer == 0 )
        return 0;

    // figure out how many bytes to transfer
    rxCount = (byteCount < bytesInBuffer) ? byteCount : bytesInBuffer;

    // transfer data to caller's buffer
    for ( i = 0; i < rxCount; i++ )
    {
        data[i] = uart_buffer[outIndex++];  // get byte from input buffer to caller's buffer

        if ( outIndex == UART_BUFF_LEN )    // update circular buffer out index
            outIndex = 0;

        bytesInBuffer--;                    // decrement byte count
    }

    return rxCount;
}

/* ---------------------------------------------------------------------------
 * uart_tx_data()
 *
 * function write 'byteCount' data bytes to the UART transmitter.
 * function blocks until all bytes have been written
 *
 */
void uart_tx_data(uint8_t *data, uint8_t byteCount)
{
    int i;

    for (i = 0; i < byteCount; i++)
    {
        uart_tx_byte(data[i]);
    }
}

/* ----------------------------------------------------------------------------
 * uart_tx_byte()
 *
 * Send byte c down the UART Tx, wait until tx holding register is empty
 *
 */
void uart_tx_byte(uint8_t c)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

/* ----------------------------------------------------------------------------
 * uart_rx_byte()
 *
 * Get byte from UART buffer
 * Note: call uart_isbyte() before this function to verify
 *       if there is any byte to read
 *
 */
uint8_t uart_rx_byte(void)
{
    uint8_t byte = 0;

    if ( bytesInBuffer > 0 )
    {
        byte = uart_buffer[outIndex++];     // if buffer has data, get byte from input buffer

        if ( outIndex == UART_BUFF_LEN )    // update circular buffer out index
            outIndex = 0;

        bytesInBuffer--;                    // decrement byte count
    }

    return byte;
}

/* ----------------------------------------------------------------------------
 * uart_isbyte()
 *
 * Test UART buffer for unread bytes, return available characters to read
 *
 */
int uart_isbyte(void)
{
    return bytesInBuffer;
}

/* ----------------------------------------------------------------------------
 * uart_flush()
 *
 * Flush UART input buffer
 *
 */
void uart_flush(void)
{
    inIndex = 0;
    outIndex = 0;
    bytesInBuffer = 0;
}

/* ----------------------------------------------------------------------------
 * uart_rx_on()
 *
 * Enable UART receiver.
 * Note: input buffers are flushed.
 *
 */
void uart_rx_on(void)
{
    /* Wait for Tx shift register to finish transmitting
     * and manually clear TXC0 bit because we have no interrupts
     * on transmit complete.
     */
    loop_until_bit_is_set(UCSR0A, TXC0);
    UCSR0A |= _BV(TXC0);

    /* Now enable the receiver because there
     * are no more bits on the half duplex line
     */
    uart_flush();
    UCSR0B |= (_BV(RXCIE0) | _BV(RXEN0));
}

/* ----------------------------------------------------------------------------
 * uart_rx_off()
 *
 * Disable UART receiver.
 *
 */
void uart_rx_off(void)
{
    UCSR0B &= ((~_BV(RXCIE0)) & (~_BV(RXEN0))); // disable Rx
    uart_flush();
}

/* ----------------------------------------------------------------------------
 * This ISR will trigger when the UART0 receives a data byte
 * Data is read and stored in a circular buffer.
 * Data can be polled and read from the circular buffer using
 * uart_getchr() or uart_rx_data()
 *
 */
ISR(USART_RX_vect)
{
    uint8_t byte;

    byte = UDR0;    // read byte to remove byte from buffer

    if ( bytesInBuffer < UART_BUFF_LEN )
    {
        uart_buffer[inIndex++] = byte;  // if buffer has free space, store byte in buffer

        if ( inIndex == UART_BUFF_LEN ) // update circular buffer index
            inIndex = 0;

        bytesInBuffer++;                // increment byte count
    }
}

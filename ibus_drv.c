/*****************************************************************************
* ibus_drv.c
*
* Source code for iBus driver.
*
* AVR UART0 interface connection to
* a half-duplex device: sensor bus of FlySky RC receiver.
*
* Modified: June 2024
* Created: September 2022
*
*****************************************************************************/

#include    <avr/io.h>
#include    <avr/interrupt.h>

#include    "util.h"
#include    "ibus_drv.h"

/****************************************************************************
  Types
****************************************************************************/

/****************************************************************************
  Definitions
****************************************************************************/
#define     IBUS_MAX_PACKET_SIZE    8
#define     IBUS_RCV_PACKET_SIZE    4
#define     IBUS_BASE_PACKET_SIZE   4

/****************************************************************************
  Globals
****************************************************************************/
uint8_t     packet_buffer[IBUS_MAX_PACKET_SIZE];
volatile    int inIndex = 0;
volatile    int isGap = 0;

/****************************************************************************
  Module functions
****************************************************************************/
static void uart_tx_data(uint8_t *, uint8_t);
static void uart_rx_on(void);
static void uart_rx_off(void);

/* ---------------------------------------------------------------------------
 * ibus_get_packet()
 *
 * Read packet from serial bus and return command and sensor ID
 *
 * Param:  pointer to received command and received sensor ID
 * Return: '-1'=packet ok, '0'=bad checksum
 *
 */
int ibus_get_packet(uint8_t *ibus_cmd, uint8_t *ibus_sensor_id)
{
    uint16_t    packet_checksum;
    uint16_t    checksum = 65535;

    /* Wait for a full 4-byte data packet from the RC receiver.
     * The gap watch-dog timer will help align packet bytes.
     */
    inIndex = 0;

    while ( inIndex < IBUS_RCV_PACKET_SIZE )
    {
        /* Do nothing */
    }

    /* Compare checksum against packet
     * fail if not the same
     */
    checksum -= packet_buffer[0];
    checksum -= packet_buffer[1];
    packet_checksum = packet_buffer[2] + (packet_buffer[3] << 8);

    if ( checksum != packet_checksum )
    {
        return IBUS_CHECKSUM_ERR;
    }

    /* Collect packet command parameters
     */
    *ibus_cmd = (packet_buffer[1] >> 4) & 0x0f;
    *ibus_sensor_id = packet_buffer[1] & 0x0f;

    return IBUS_PACKET_OK;
}

/* ---------------------------------------------------------------------------
 * ibus_send_packet()
 *
 * Build and send an iBus packet to the RC receiver.
 *
 * Param:  pointer to packet content structure and data byte count
 * Return: nothing
 *
 */
void ibus_send_packet(ibus_packet_t *packet, int data_count)
{
    int         i, s;
    uint16_t    checksum;

    /* Build packet buffer from input parameters
     */
    checksum = (packet->ibus_cmd << 4) + (packet->ibus_sense_id & 0x0f);
    packet_buffer[1] = checksum;

    for ( i = 0; i < data_count; i++ )
    {
        packet_buffer[2 + i] = packet->data[i];
        checksum += packet->data[i];
    }

    s = IBUS_BASE_PACKET_SIZE + data_count;
    packet_buffer[0] = s;
    checksum += s;

    checksum = 65535 - checksum;
    packet_buffer[2 + i] = (uint8_t)(checksum & 0x00ff);
    packet_buffer[3 + i] = (uint8_t)(checksum >> 8);

    /* Transmit packet
     */
    uart_rx_off();
    uart_tx_data(packet_buffer, s);
    uart_rx_on();
}

/* ---------------------------------------------------------------------------
 * uart_tx_data()
 *
 * Write 'byteCount' data bytes to the UART transmitter.
 * Function blocks until all bytes have been written
 *
 * Param:  pointer to data buffer and byte count
 * Return: none
 * 
 */
static void uart_tx_data(uint8_t *data, uint8_t byteCount)
{
    int i;

    for (i = 0; i < byteCount; i++)
    {
        loop_until_bit_is_set(UCSR0A, UDRE0);
        UDR0 = data[i];
    }
}

/* ----------------------------------------------------------------------------
 * uart_rx_on()
 *
 *  Enable UART receiver.
 *
 * Param:  none
 * Return: none
 * 
 */
static void uart_rx_on(void)
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
    UCSR0B |= (_BV(RXCIE0) | _BV(RXEN0));
}

/* ----------------------------------------------------------------------------
 * uart_rx_off()
 *
 *  Disable UART receiver.
 *
 * Param:  none
 * Return: none
 * 
 */
static void uart_rx_off(void)
{
    UCSR0B &= ((~_BV(RXCIE0)) & (~_BV(RXEN0)));
}

/* ----------------------------------------------------------------------------
 * This ISR will trigger when the UART0 receives a data byte
 * from the RC Receiver.
 *
 */
ISR(USART_RX_vect)
{
    if ( isGap )
    {
        /* If 'isGap' is true then this is the first byte  of a packet
           received from the RC receiver.
        */
        isGap = 0;
        inIndex = 0;
    }

    if ( inIndex < IBUS_MAX_PACKET_SIZE )
    {
        packet_buffer[inIndex] = UDR0;
        inIndex++;
    }

    /* Reset the gap watchdog timer
     */
    enable_gap_timer();
}

/* ----------------------------------------------------------------------------
 * This ISR will trigger when the byte timer expires.
 * If this timer triggers then no byte has been received for at least one byte-time duration
 * after the last byte was receive, and a transmission gap is in effect.
 *
 */
ISR(TIMER1_COMPA_vect)
{
    isGap = 1;
    disable_gap_timer();
}

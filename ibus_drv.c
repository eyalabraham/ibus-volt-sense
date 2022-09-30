/*****************************************************************************
* ibus_drv.c
*
* Driver source for iBus driver.
*
* Created: September 2022
*
*****************************************************************************/

#include    "uart_hduplex_drv.h"
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
  Function prototypes
****************************************************************************/

/****************************************************************************
  Globals
****************************************************************************/
uint8_t     packet_buffer[IBUS_MAX_PACKET_SIZE];

/****************************************************************************
  Module functions
****************************************************************************/

/* ---------------------------------------------------------------------------
 * ibus_get_packet()
 *
 * Read packet from serial bus and return command and sensor ID
 *
 * Param:  pointer to received command and received sensor ID
 * Return: '-1'=packet ok, '-2'=retry, '0'=bad checksum
 *
 */
int ibus_get_packet(uint8_t *ibus_cmd, uint8_t *ibus_sensor_id)
{
    int         packet_length;
    uint16_t    packet_checksum;
    uint16_t    checksum;

    /* Make sure that the buffer is big enough and that there
     * is at least one packet in the serial input buffer
     */
    if ( uart_isbyte() < IBUS_RCV_PACKET_SIZE )
        return IBUS_READ_RETRY;

    /* Collect a data packet
     */
    uart_rx_data(packet_buffer, IBUS_RCV_PACKET_SIZE);

    /* Collect packet command parameters
     */
    packet_length = (int)packet_buffer[0];
    checksum = packet_length;

    *ibus_cmd = (packet_buffer[1] >> 4) & 0x0f;
    *ibus_sensor_id = packet_buffer[1] & 0x0f;
    checksum += packet_buffer[1];

    packet_checksum = packet_buffer[2] + (packet_buffer[3] << 8);

    /* Compare checksum against packet
     * fail if not the same
     */
    checksum = 65535 - checksum;
    if ( checksum != packet_checksum )
        return IBUS_CHECKSUM_ERR;

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


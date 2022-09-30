/*****************************************************************************
* ibus_drv.h
*
* iBus driver header file
*
* Created: September 2022
*
*****************************************************************************/

#ifndef __IBUS_DRV_H__
#define __IBUS_DRV_H__

#include    <stdint.h>

/****************************************************************************
  Definitions
****************************************************************************/
#define     IBUS_CMD_DISCOVER       8
#define     IBUS_CMD_SENSOR_TYPE    9
#define     IBUS_CMD_SENSOR_READ   10

#define     IBUS_CHECKSUM_ERR       0
#define     IBUS_PACKET_OK         -1
#define     IBUS_READ_RETRY        -2

typedef struct {
    uint8_t     ibus_cmd;
    uint8_t     ibus_sense_id;
    uint8_t     data[4];
} ibus_packet_t;

/****************************************************************************
  Function prototypes
****************************************************************************/

int     ibus_get_packet(uint8_t *ibus_cmd, uint8_t *ibus_sensor_id);
void    ibus_send_packet(ibus_packet_t *packet, int data_count);

#endif  /* __IBUS_DRV_H__ */

/*
 * ibusvsense.c
 *
 *  This program runs the FlySky-compatible i.BUS voltage sensor
 *  implemented in an AVR ATmega328P.
 *
 * ATmega328P AVR IO
 *
 * Port B bit assignment
 *
 *  b7 b6 b5 b4 b3 b2 b1 b0
 *  |  |  |  |  |  |  |  |
 *  |  |  |  |  |  |  |  +--- 'o' Status LED(*) (active low)
 *  |  |  |  |  |  |  +------ 'i'
 *  |  |  |  |  |  +--------- 'i'
 *  |  |  |  |  +------------ 'o' MOSI \
 *  |  |  |  +--------------- 'i' MISO  | In-circuit programmer
 *  |  |  +------------------ 'o' SCLK /
 *  |  +--------------------- 'i' Xtal1 (not used)
 *  +------------------------ 'i' Xtal2 (not used)
 *
 * (*) 'on'= packets being received, 'off'=no packets being received, 'blink'=packets with bad CRC
 *
 * Port C bit assignment
 *
 *     b6 b5 b4 b3 b2 b1 b0
 *     |  |  |  |  |  |  |
 *     |  |  |  |  |  |  +--- 'i' ADC0 analog input (battery sense voltage)
 *     |  |  |  |  |  +------ 'i'
 *     |  |  |  |  +--------- 'i'
 *     |  |  |  +------------ 'i'
 *     |  |  +--------------- 'i'
 *     |  +------------------ 'i'
 *     +--------------------- 'i' ^Reset (with pull up)
 *
 * Port D bit assignment
 *
 *  b7 b6 b5 b4 b3 b2 b1 b0
 *  |  |  |  |  |  |  |  |
 *  |  |  |  |  |  |  |  +--- 'i' UART Rx
 *  |  |  |  |  |  |  +------ 'o' UART Tx
 *  |  |  |  |  |  +--------- 'i'
 *  |  |  |  |  +------------ 'i'
 *  |  |  |  +--------------- 'i'
 *  |  |  +------------------ 'i'
 *  |  +--------------------- 'i'
 *  +------------------------ 'i'
 *
 * Note: all references to data sheet are for ATnega328 Rev. 8161D–AVR–10/09
 *
 */

#include    <stdint.h>

#include    <avr/io.h>
#include    <avr/interrupt.h>
#include    <util/delay.h>

#include    "util.h"
#include    "ibus_drv.h"
#include    "sensor_type.h"
#include    "uart_hduplex_drv.h"

/****************************************************************************
  Definitions
****************************************************************************/
#define     LED_OFF             0
#define     LED_FLASH           1
#define     LED_ON              2

#define     BATT_2S             0
#define     BATT_3S             1
#define     BATT_4S             2

#define     BATT_SIZES          3
#define     BATT_PERCENTS       21
#define     DEF_BATTERY_PERCENT 100

#define     NO_PACKET_TIME_OUT  (1*RATE_1HZ)    // 1 second
#define     STARTUP_DELAY       (2*RATE_1HZ)    // 2 seconds

/****************************************************************************
  Function prototypes
****************************************************************************/
uint8_t     get_battery_percent(uint16_t adc_value);

/****************************************************************************
  Globals
****************************************************************************/
uint16_t    startup_time_mark;

uint16_t    battery_capacity[BATT_PERCENTS][BATT_SIZES] =
{
/* Values are fixed point at 0.01v per LSB
 *
 *        2S    3S    4S
 */
        { 655,  982, 1309 },    // 0%
        { 722, 1083, 1443 },    // 5%
        { 737, 1106, 1475 },    // 10%
        { 741, 1112, 1483 },    // 15%
        { 745, 1118, 1491 },    // 20% << discharge danger point
        { 749, 1124, 1499 },    // 25%
        { 753, 1130, 1506 },    // 30%
        { 757, 1136, 1514 },    // 35%
        { 759, 1139, 1518 },    // 40%
        { 763, 1145, 1526 },    // 45%
        { 767, 1151, 1534 },    // 50%
        { 771, 1156, 1542 },    // 55%
        { 775, 1162, 1550 },    // 60%
        { 783, 1174, 1566 },    // 65%
        { 791, 1186, 1581 },    // 70%
        { 797, 1195, 1593 },    // 75%
        { 805, 1207, 1609 },    // 80%
        { 816, 1225, 1633 },    // 85%
        { 822, 1233, 1645 },    // 90%
        { 830, 1245, 1660 },    // 95%
        { 840, 1260, 1680 },    // 100%
};

/* ----------------------------------------------------------------------------
 * main() control functions
 *
 */
int main(void)
{
    uint16_t        time_mark;
    uint16_t        no_packet_time_mark;
    uint8_t         battery_percent;
    int             ibus_result;
    uint8_t         ibus_cmd, ibus_sensor_id;
    ibus_packet_t   packet;

    int             led_state = LED_OFF;
    uint16_t        adc_value = 0;

    /* Initialize IO devices and
     * enable interrupts
     */
    ioinit();
    sei();

    time_mark = get_global_time();
    no_packet_time_mark = time_mark;
    startup_time_mark = time_mark;

    /* Loop forever
     */
    while ( 1 )
    {
        /* Read the ibus and process packets
         */
        ibus_result = ibus_get_packet(&ibus_cmd, &ibus_sensor_id);

        if ( ibus_result == IBUS_PACKET_OK )
        {
            packet.ibus_cmd = ibus_cmd;
            packet.ibus_sense_id = ibus_sensor_id;

            if ( ibus_cmd == IBUS_CMD_DISCOVER )
            {
                if ( ibus_sensor_id == 1 ||
                     ibus_sensor_id == 2    )
                {
                    ibus_send_packet(&packet, 0);
                }
            }
            else if ( ibus_cmd == IBUS_CMD_SENSOR_TYPE )
            {
                if ( ibus_sensor_id == 1 )
                {
                    packet.data[0] = IBUS_SENSOR_TYPE_EXTERNAL_VOLTAGE;
                    packet.data[1] = 2;
                    ibus_send_packet(&packet, 2);
                }
                else if ( ibus_sensor_id == 2 )
                {
                    packet.data[0] = IBUS_SENSOR_TYPE_FUEL;
                    packet.data[1] = 2;
                    ibus_send_packet(&packet, 2);
                }

            }
            else if ( ibus_cmd == IBUS_CMD_SENSOR_READ )
            {
                if ( ibus_sensor_id == 1 )
                {
                    /* Read and convert ADC reading
                     * to battery voltage in 0.01v/per MSB bit units.
                     * Calculation order is IMPORTANT in order to
                     * maintain accuracy and stay within 16-bits.
                     */
                    adc_value = get_adc();
                    adc_value *= 193;   // Voltage divider ratio (1/19.3)
                    adc_value >>= 8;    // ADC readout scaling
                    adc_value *= 106;   // AVR reference voltage (1.06v)
                    adc_value /= 10;    // Compensate for 10x value on reference voltage

                    packet.data[0] = (uint8_t)(adc_value & 0xff);
                    packet.data[1] = (uint8_t)(adc_value >> 8);
                    ibus_send_packet(&packet, 2);
                }
                else if ( ibus_sensor_id == 2 )
                {
                    /* Calculate remaining battery percent
                     */
                    battery_percent = get_battery_percent(adc_value);

                    packet.data[0] = battery_percent;
                    packet.data[1] = 0;
                    ibus_send_packet(&packet, 2);
                }
            }

            led_state = LED_ON;
        }

        /* If we get a checksum error, then we are not aligned on
         * a packet boundary. Do a dummy read and try again until
         * alignment is reestablished.
         */
        else if ( ibus_result == IBUS_CHECKSUM_ERR )
        {
            _delay_us(200);
            uart_rx_byte();
            led_state = LED_FLASH;
        }

        /* No full data packet available in receiver buffer.
         */
        else
        {
            if ( (uint16_t)(get_global_time() - no_packet_time_mark) >= NO_PACKET_TIME_OUT )
            {
                no_packet_time_mark = get_global_time();
                led_state = LED_OFF;
            }
        }


        /* LED indicator
         */
        if ( led_state == LED_ON )
        {
            status_led_on();
        }
        else if ( led_state == LED_FLASH &&
                (uint16_t)(get_global_time() - time_mark) >= RATE_4HZ )
        {
            status_led_swap();
            time_mark = get_global_time();
        }
        else if ( led_state == LED_OFF )
        {
            status_led_off();
        }
    }

    return 0;
}

/* ----------------------------------------------------------------------------
 * get_battery_percent()
 *
 *  Convert ADC voltage (fixed point 0.01v per LSB) to battery percent
 *
 *  param:  ADC readout
 *  return: battery percent 0 to 100
 *
 */
uint8_t get_battery_percent(uint16_t adc_value)
{
    int         x;

    static int  startup_delay = 1;
    int  battery_size = -1;

    /* Return a bogus battery full for STARTUP_DELAY seconds
     * until system stabilizes.
     */
    if ( startup_delay &&
         (uint16_t)(get_global_time() - startup_time_mark) < STARTUP_DELAY )
    {
        return DEF_BATTERY_PERCENT;
    }

    startup_delay = 0;

    /* Determine battery size
     * and then battery percent
     */
    for ( x = 0; x < BATT_SIZES; x++ )
    {
        if ( adc_value >= battery_capacity[0][x] &&
             adc_value <= battery_capacity[20][x] )
        {
            battery_size = x;
            break;
        }
    }

    if ( battery_size != -1 )
    {
        for ( x = 0; x < (BATT_PERCENTS-1) ; x++ )
        {
            if ( adc_value > battery_capacity[x][battery_size] &&
                 adc_value <= battery_capacity[(x + 1)][battery_size] )
            {
                return (x * 5);
            }
        }
    }

    return 0;
}

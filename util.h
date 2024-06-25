/*****************************************************************************
* util.h
*
* Utility functions
*
* Modified: June 2024
* Created: September 2022
*
*****************************************************************************/

#ifndef __UTIL_H__
#define __UTIL_H__

/****************************************************************************
  Definitions
****************************************************************************/
// IO port B initialization
#define     PB_DDR_INIT     0b00101001  // Port data direction
#define     PB_PUP_INIT     0b00000000  // Port input pin pull-up
#define     PB_INIT         0b00000001  // Port initial values

#define     STATUS_LED      0b00000001
#define     CALIBRATION     0b00000010

/* IO port C initialization
 */
#define     PC_DDR_INIT     0b00000000  // Port data direction
#define     PC_PUP_INIT     0b00000000  // Port input pin pull-up
#define     PC_INIT         0b00000000  // Port initial values

/* IO port D initialization
 */
#define     PD_DDR_INIT     0b00000010  // Port data direction
#define     PD_PUP_INIT     0b00000000  // Port input pin pull-up
#define     PD_INIT         0b00000010  // Port initial values

/* Timer0 initialization
 * Using Timer0 for time-out measurements
 * Divide system clock by 1,024, and Timer0 Mode-0 by 256
 * for overflow interrupt interval
 */
#define     TCCR0A_INIT     0b00000000  // Normal mode, no compare match
#define     TCCR0B_INIT     0b00000101  // Mode-0 timer, Clk/1024
#define     TIMSK_INIT      0b00000001  // Enable Timer0 overflow interrupt

#if (F_CPU == 8000000UL)
#define     RATE_1HZ        30          // Equivalent timer ticks
#define     RATE_2HZ        15
#define     RATE_4HZ        7
#elif (F_CPU == 10000000UL)
#define     RATE_1HZ        38          // Equivalent timer ticks
#define     RATE_2HZ        19
#define     RATE_4HZ        9
#elif (F_CPU == 16000000UL)
#define     RATE_1HZ        61          // Equivalent timer ticks
#define     RATE_2HZ        30
#define     RATE_4HZ        15
#else
#warning "Device clock not defined"
#endif

/* Timer1 initialization
 */
#define     OCR1AL_INIT     156;        // 1mSec time out.
#define     TCCR1A_INIT     0b00000000  // Set timer top with OCR1A
#define     TCCR1B_INIT     0b00001011  // Fosc 1/64
#define     TCCR1C_INIT     0b00000000
#define     TIMSK1_INIT     0b00000010

#define     TMR1_DIS        0b11111000  // AND with TCCR1B
#define     TMR1_ENA        0b00000011  // OR with TCCR1B to enable timer at Fosc 1/64

/* UART BAUD rates
 */
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

/* ADC converter
 */
#define     ADMUX_INIT      0b00100000; // External reference, left adjusted result, ADC0 source
#define     ADCSRA_INIT     0b11101111; // Auto trigger, Fclk/128
#define     ADCSRB_INIT     0b00000100; // Timer/Counter0 Overflow trigger source ~38Hz
#define     DIDR0_INIT      0b00000001; // disable digital input on ADC0

/****************************************************************************
  Function prototypes
****************************************************************************/
void     reset(void) __attribute__((naked)) __attribute__((section(".init3")));
void     ioinit(void);
void     enable_gap_timer(void);
void     disable_gap_timer(void);
void     status_led_on(void);
void     status_led_off(void);
void     status_led_swap(void);
uint16_t get_adc(void);
uint16_t get_global_time(void);

#endif /* __UTIL_H__ */

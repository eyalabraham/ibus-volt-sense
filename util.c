/*****************************************************************************
* util.c
*
* Utility functions
*
* Modified: June 2024
* Created: September 2022
*
*****************************************************************************/

#include    <avr/io.h>
#include    <avr/interrupt.h>
#include    <avr/wdt.h>

#include    "util.h"

/****************************************************************************
  Types and definitions
****************************************************************************/

/* ADC readout averaging parameters.
 * ADC is read at Timer0 rate, with Fclk at 10Mhz the rate
 * is about 38 samples per second. Selecting ADC_AVERAGE_BITS=5
 * will result in averaging 32 samples (approximately 1 second).
 */
#define     ADC_AVERAGE_BITS    5                       // 1, 2, 3, 4, or 5
#define     ADC_AVERAGE_MAX     32
#define     ADC_AVERAGE         (1<<ADC_AVERAGE_BITS)   // Power of 2 (2, 4, 8, 16, 32)
#if (ADC_AVERAGE>ADC_AVERAGE_MAX)
#warning "ADC averaging is out of range. Reduce ADC_AVERAGE_BITS!"
#endif

/****************************************************************************
  Function prototypes
****************************************************************************/

/****************************************************************************
  Globals
****************************************************************************/
volatile uint16_t   global_counter = 0;     // Global time base
volatile uint16_t   adc = 0;                // Global ADC last value

/* ----------------------------------------------------------------------------
 * reset()
 *
 *  Clear SREG_I on hardware reset.
 *  source: http://electronics.stackexchange.com/questions/117288/watchdog-timer-issue-avr-atmega324pa
 */
void reset(void)
{
     cli();
    // Note that for newer devices (any AVR that has the option to also
    // generate WDT interrupts), the watchdog timer remains active even
    // after a system reset (except a power-on condition), using the fastest
    // prescaler value (approximately 15 ms). It is therefore required
    // to turn off the watchdog early during program startup.
    MCUSR = 0;  // clear reset flags
    wdt_disable();
}

/* ----------------------------------------------------------------------------
 * ioinit()
 *
 *  Initialize IO interfaces.
 *
 */
void ioinit(void)
{
    /* Reconfigure system clock scaler
     */
    CLKPR = 0x80;   // Enable scaler        (sec 8.12.2)
    CLKPR = 0x00;   // Change clock scaler

    /* General IO pins
     */
    DDRB  = PB_DDR_INIT;
    PORTB = PB_INIT | PB_PUP_INIT;

    /* Timer0
     */
    TCNT0 = 0;
    TCCR0A = TCCR0A_INIT;
    TCCR0B = TCCR0B_INIT;
    TIMSK0 = TIMSK_INIT;

    /* Timer1, this is the 'gap timer' watchdog
     */
    TCNT1H = 0;
    TCNT1L = 0;
    OCR1AH = 0;
    OCR1AL = OCR1AL_INIT;
    TCCR1A = TCCR1A_INIT;
    TCCR1B = (TCCR1B_INIT & TMR1_DIS);
    TCCR1C = TCCR1C_INIT;
    TIMSK1 = TIMSK1_INIT;

    /* Setup the UART
     */
    UCSR0A = _BV(U2X0);                 // Double baud rate (sec 19.10 p.195)
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 8 data bits, 1 stop, no parity
    UBRR0 = BAUD_115200;

    UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);   // enable Tx and Rx

    /* ADC
     */
    ADMUX = ADMUX_INIT;
    ADCSRB = ADCSRB_INIT;
    DIDR0 = DIDR0_INIT;
    ADCSRA = ADCSRA_INIT;
}

/* ----------------------------------------------------------------------------
 * enable_gap_timer()
 *
 *  Enable the gap timer
 * 
 *  param:  none
 *  return: none
 *
 */
void enable_gap_timer(void)
{
    TCNT1H = 0;
    TCNT1L = 0;
    TCCR1B |= TMR1_ENA;
}

/* ----------------------------------------------------------------------------
 * disable_gap_timer()
 *
 *  Disable the gap timer
 * 
 *  param:  none
 *  return: none
 *
 */
void disable_gap_timer(void)
{
    TCCR1B &= TMR1_DIS;
}

/* ----------------------------------------------------------------------------
 * status_led_on()
 *
 *  Turn on LED
 *
 *  param:  none
 *  return: none
 *
 */
void status_led_on(void)
{
    PORTB &= ~STATUS_LED;
}

/* ----------------------------------------------------------------------------
 * status_led_off()
 *
 *  Turn LED off
 *
 *  param:  none
 *  return: none
 *
 */
void status_led_off(void)
{
    PORTB |= STATUS_LED;
}

/* ----------------------------------------------------------------------------
 * status_led_swap()
 *
 *  Swap LED state on <-> off
 *
 *  param:  none
 *  return: none
 *
 */
void status_led_swap(void)
{
    PORTB ^= STATUS_LED;
}

/* ----------------------------------------------------------------------------
 * get_adc()
 *
 *  Return the value of the last ADC conversion
 *
 *  param:  none
 *  return: ADC value
 *
 */
uint16_t get_adc(void)
{
    return adc;
}

/* ----------------------------------------------------------------------------
 * get_global_time()
 *
 *  Return the value of the global timer tick counter
 *
 *  param:  none
 *  return: counter value
 *
 */
uint16_t get_global_time(void)
{
    return global_counter;
}

/* ----------------------------------------------------------------------------
 * This ISR will trigger when ADC0 analog to digital conversion is complete.
 * The ISR implements a moving average calculation over the ADC readouts
 * using a circular buffer algorithm.
 *
 */
ISR(ADC_vect)
{
    uint8_t         adc_readout;

    static uint16_t adc_values[ADC_AVERAGE_MAX] =
            { 0,0,0,0,0,0,0,0,0,0,
              0,0,0,0,0,0,0,0,0,0,
              0,0,0,0,0,0,0,0,0,0,
              0,0 };
    static uint8_t  adc_in = (ADC_AVERAGE - 1);
    static uint8_t  adc_out = 0;
    static uint16_t adc_sum = 0;

    adc_readout = ADCH;

    adc_sum -= adc_values[adc_out];
    adc_out++;
    adc_out &= (ADC_AVERAGE - 1);

    adc_in++;
    adc_in &= (ADC_AVERAGE - 1);
    adc_values[adc_in] = adc_readout;
    adc_sum += adc_readout;

    adc = (adc_sum >> ADC_AVERAGE_BITS);
}

/* ----------------------------------------------------------------------------
 * This ISR will trigger approximately every 26mSec when Timer0 overflows, @ 10MHz clock.
 * The ISR increments a global 16-bit time variable that will overflow (cycle back through 0)
 * approximately every 28.6 minutes.
 *
 */
ISR(TIMER0_OVF_vect)
{
    global_counter++;
}

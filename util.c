/*****************************************************************************
* util.c
*
* Utility functions
*
* Created: September 2022
*
*****************************************************************************/

#include    <avr/io.h>
#include    <avr/interrupt.h>
#include    <avr/wdt.h>

#include    "util.h"
#include    "uart_hduplex_drv.h"

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

    /* USART
     */
    uart_initialize(UART_BAUD_115200);

    /* ADC
     */
    ADMUX = ADMUX_INIT;
    ADCSRB = ADCSRB_INIT;
    DIDR0 = DIDR0_INIT;
    ADCSRA = ADCSRA_INIT;
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
 * get_global_time()
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
 * This ISR will trigger approximately every 33mSec when Timer0 overflows.
 * The ISR increments a global 8-bit time variable that will overflow (cycle back through 00)
 * approximately every 8.4 seconds.
 *
 */
ISR(TIMER0_OVF_vect)
{
    global_counter++;
}

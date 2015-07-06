/*
 * for hardware randomizer connected between
 * AIN1 (PD7, digital pin 7) and
 * AIN0 (PD6, digital pin 6)
 * of Arduino Duemilanove hardware (ATmega168)
 * by Kenji Rikitake 9-MAR-2009
 */

/* #define F_CPU (16000000UL) */

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/* initialize IO ports */
static void ioinit(void) {

    /* disable interrupt before initialization*/
    cli();

    /*
     * forced initialization
     * of unnecessary interrupts
     */

    MCUCR = 0x00;
    EICRA = 0x00;
    EIMSK = 0x00;
    TIMSK0 = 0x00;
    TIMSK1 = 0x00;
    TIMSK2 = 0x00;
    PCMSK0 = 0x00;
    PCMSK1 = 0x00;
    PCMSK2 = 0x00;
    PCICR = 0x00;
    PRR = 0x00;

    /*
     * AIN1/PD7/Pin 7 to input
     * AIN0/PD6/Pin 6 to input
     * TXD/PD1/Pin 1 to output
     * RXD/PD0/Pin 0 to input 
     * AIN1/AIN0 pullup off
     */

    DDRD = 0x3e;
    PORTD = 0x3f;

    DDRC = 0xff;
    PORTC = 0xff;

    /* PB5 = LED on Arduino 2009 */
    DDRB = 0xff;
    PORTB = 0x3f;

    /* comparator enabled, use AIN0, no interrupt */
    ACSR = 0x00;

    /* USART 0 */
    /* no USART IRQ, disable TX/RX */
    UCSR0B = 0x00;
    /* clk 2x */
    UCSR0A = 0x00;
    /* async, no parity, 8bit */
    UCSR0C = 0x06;
    /* baud rate (115200bps, 16MHz, 2x) */
    /* real rate: 111111bps (9 microsecs/bit) */
    UBRR0H = 0;
    UBRR0L = 8;
    /* no USART IRQ, enable TX (no RX) */
    UCSR0B = 0x08;

    /* Timer 0 */
    /* timer stop */
    TCCR0B = 0x00;
    /* reset counter */
    TCNT0 = 0;
    /* no external output, CTC */
    TCCR0A = 0x02;
    /*
     * Idealistically, keep
     * (timer period) * 16 > (baud rate period * 10)
     * to maintain the condition
     * that every generated random byte
     * need not be buffered.
     * (min. 16 random source samplings are required
     *  and sending 10 bits to serial port are required)
     * On the other hand, making timer period shorter
     * may not cause real harm, because:
     * * The raw analog comparator output is mostly zero bits
     * * busy waiting of the USART will prevent output overrun
     */
    /*
     * The noise characteristics do not differ
     * Between the raw comparator sampling rates of
     * 1us to 4096us
     */
    /*
     * Setting the timer period less than 750ns
     * may cause the comparator to delay or
     * unable to follow
     */
    /* timer period: 12/16 = 0.75 microseconds */
    OCR0A = (12) - 1;
    /* clk (0.06125 microseconds / count, or 16MHz) */
    /* start timer */
    TCCR0B = 0x01;

    /* enable OCIE0A interrupt */
    TIMSK0 = 0x02;

    /* enable interrupt after initialization*/
    sei();
}

/* output to USART0 */

static void putchr(uint8_t c) {
    /*
     * busy-waiting UDRE0
     * note: 
     * if sampling byte generation rate is slower
     * than the byte sending rate to serial,
     * busy-waiting period will not happen
     * many times, so this doesn't have to be
     * interrupt-driven
     */
    loop_until_bit_is_set(UCSR0A, UDRE0);
    /* Output the given char to USART0 */
    UDR0 = c;
}

/* shared variables between timer ISR and main() */
/* volatile qualifier required */

/*
 * flagandbit states:
 * 0: no data
 * 2: obtained '0'
 * 3: obtained '1'
 */

volatile uint8_t flagandbit = 0;

/*
 * sampling from the random source
 * with the timer0 COMPA ISR
 */

#define comparator_input() (ACSR & _BV(ACO))

ISR(TIMER0_COMPA_vect) {
    static uint8_t state = 0;
    static uint8_t oval = 0;

    /* do nothing when flag is set */
    if (flagandbit != 0)
        return;
    /*
     * if flag is not set,
     * check the input state
     */
    /*
     * state: 0 -> 1st bit
     * state: 1 -> 2nd bit
     */
    if (state == 0) {
        /* save the 1st bit to oval */
        state++;
        oval = comparator_input();
    } else {
        /* clear the state */
        state = 0;
        /* obtain the 2nd bit */
        /* do nothing if two bits are the same */
        if (comparator_input() != oval) {
            if (oval != 0) {
                /* {1st, 2nd} = {1, 0} */
                /* Turn on the LED */
                PORTB |= 0x20;
                /* Set the output */
                flagandbit = 3;
            } else {
                /* {1st, 2nd} = {0, 1} */
                /* Turn off the LED */
                PORTB &= 0xdf;
                /* Set the output */
                flagandbit = 2;
            }
        }
    }
}

/* main routine */

int main() {
    uint8_t i = 0;
    uint8_t p = 0;
    uint8_t j = 0;
    uint8_t p2 = 0;
    uint8_t p2b = 0;

    /* initialize ports, timers, serial, and IRQ */
    ioinit();

    for (;;) {
        /* check whether a random bit is found */
        if (flagandbit != 0) {
            /* accumulate 8 bits */
            p = p + p;
            if ((flagandbit & 0x01) != 0) {
                p++;
            }
            i++;
            /* unlock the timer IRQ flags */
            flagandbit = 0;
            /* print accumulated value */
            if (i > 7) {
                p2 = p;
                /* clear counters and buffer */
                p = 0;
                i = 0;
                /* change byte xor machine state */
                j++;
                if (j > 1) {
                    j = 0;
                    /* and output the result in a raw byte*/
                    putchr(p2b ^ p2);
                } else {
                    /* save the output byte and keep on generating */
                    p2b = p2;
                }
            }
        }
    }
    /* NOTREACHED */
    return 0;
}

/* end of code */

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2009-2015 Kenji Rikitake
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/*
 * for hardware randomizer connected between
 * PD7 (digital pin 7) and
 * PD6 (digital pin 6)
 * of Arduino Duemilanove hardware (ATmega168)
 * by Kenji Rikitake
 */

/* #define F_CPU (16000000UL) */

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

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
     * PDx:
     * PD7/Pin 7 to input
     * PD6/Pin 6 to input
     * TXD/PD1/Pin 1 to output
     * RXD/PD0/Pin 0 to input
     * pullup enabled
     */
    DDRD = 0x3e;
    PORTD = 0xff;

    /* PCx all output, pulled up */
    DDRC = 0xff;
    PORTC = 0xff;

    /* PB5 = LED on Arduino 2009 */
    DDRB = 0xff;
    PORTB = 0x3f;

    /* comparator disabled */
    ACSR = 0x80;

    /* USART 0 */
    /* no USART IRQ, disable TX/RX */
    UCSR0B = 0x00;
    /* clk 1x */
    UCSR0A = 0x00;
    /* async, no parity, 8bit */
    UCSR0C = 0x06;
    /* baud rate (11520bps, 16MHz, U2Xn=1) */
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
     * (timer period) * 32 > (baud rate period * 10)
     * to maintain the condition that
     * every generated random byte need not be buffered.
     * (Minimum 32 random source samplings are required
     *  for generating a byte of random number output
     *  and sending 10 bits to serial port are required.)
     * On the other hand, making timer period shorter
     * may not cause real harm, because:
     * * The raw analog comparator output is mostly zero bits
     * * busy waiting of the USART will prevent output overrun
     */
    /*
     * The noise characteristics do not differ
     * between the raw comparator sampling rates of
     * 1us to 4096us
     */
    /*
     * Setting the timer period too small
     * may break the timer-driven interrupt routine
     */
    /* timer period: 5 microseconds = 80 machine cycles */
    OCR0A = (5 * 2) - 1;
    /* clk/8 (0.5 microseconds / count) */
    /* start timer */
    TCCR0B = 0x02;

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
 * sampling from the random source
 * with the timer0 COMPA ISR
 * Note: this ISR code is *interrupt driven*
 */

/*
 * declare all ISR variables
 * as volatile and mapped into AVR registers
 */
volatile register uint8_t samplecheck asm("r16");
volatile register uint8_t sval asm("r15");

ISR(TIMER0_COMPA_vect) {
    if (samplecheck != 0) {
        return;    
    }
    /* sample the value from Port D */
    sval = PIND >> 6;
    samplecheck = 1;
}


/* main function */

int main() {
    uint8_t i = 0;
    uint8_t p = 0;
    uint8_t j = 0;
    uint8_t p2 = 0;
    uint8_t p2b = 0;
    uint8_t state = 0;
    uint8_t oval0 = 0;
    uint8_t oval1 = 0;
    uint8_t flagandbit0 = 0;
    uint8_t flagandbit1 = 0;

    /* initialize ports, timers, serial, and IRQ */
    ioinit();

    /* initialize the global register variables */
    samplecheck = 0;
    sval = 0;

    for (;;) {
        if (samplecheck != 0) {
            /* Check sval bit pair */
            if (state == 0) {
                /* save the 1st bit pair to oval[01] */
                oval0 = sval & 0x01;
                oval1 = sval & 0x02;
                /* set the 2nd bit state */
                state = 1;
            } else {
                /* obtain the 2nd bit pair */
                /* do nothing if two bits are the same */
                if ((sval & 0x01) != oval0) {
                    if (oval0 == 0) {
                        /* {1st, 2nd} = {0, 1} */
                        flagandbit0 = 1;
                    } else {
                        /* {1st, 2nd} = {1, 0} */
                        flagandbit0 = 2;
                    }
                }
                /* obtain the 2nd bit */
                /* do nothing if two bits are the same */
                if ((sval & 0x02) != oval1) {
                    if (oval1 == 0) {
                        /* {1st, 2nd} = {0, 1} */
                        flagandbit1 = 1;
                    } else {
                        /* {1st, 2nd} = {1, 0} */
                        flagandbit1 = 2;
                    }
                }
                /* set the 1st bit state */
                state = 0;
            }
            /* reset the interrupt allowance flag */
            samplecheck = 0;
        }
        /* check whether a random bit is found */
        if (flagandbit0 != 0) {
            /* accumulate 8 bits */
            p = p + p;
            p += flagandbit0 & 0x01;
            i++;
            /* unlock the timer IRQ flags */
            flagandbit0 = 0;
        }
        /* print accumulated value */
        if (i > 7) {
            /* flip PORTB LED */
            PORTB ^= 0x40;
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
        /* check whether a random bit is found */
        if (flagandbit1 != 0) {
            /* accumulate 8 bits */
            p = p + p;
            p += flagandbit1 & 0x01;
            i++;
            /* unlock the timer IRQ flags */
            flagandbit1 = 0;
        }
        /* print accumulated value */
        if (i > 7) {
            /* flip PORTB LED */
            PORTB ^= 0x40;
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
    /* NOTREACHED */
    return 0;
}

/* end of code */

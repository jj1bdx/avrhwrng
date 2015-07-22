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
 * AIN1 (PD7, digital pin 7) and
 * AIN0 (PD6, digital pin 6)
 * of Arduino Duemilanove hardware (ATmega168)
 * by Kenji Rikitake
 */

/* #define F_CPU (16000000UL) */

/* Size optimized */
#pragma GCC optimize ("Os")

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
     * AIN1/PD7/Pin 7 to input
     * AIN0/PD6/Pin 6 to input
     * TXD/PD1/Pin 1 to output
     * RXD/PD0/Pin 0 to input 
     * AIN1/AIN0 pullup off
     */

    DDRD = 0x3e;
    PORTD = 0x3f;

    /*
     * ADC1/PC1/Pin A1 to input
     * ADC0/PC0/Pin A0 to input
     * ADC1/ADC0 pullup off
     */

    DDRC = 0xfc;
    PORTC = 0xfc;

    /* PB5 = LED on Arduino 2009 */
    DDRB = 0xff;
    PORTB = 0x3f;

    /* comparator enabled, use AIN0, no interrupt */
    ACSR = 0x00;

    /* USART 0 */
    /* no USART IRQ, disable TX/RX */
    UCSR0B = 0x00;
    /* clk 1x */
    UCSR0A = 0x00;
    /* async, no parity, 8bit */
    UCSR0C = 0x06;
    /* baud rate (19200bps, 16MHz, U2Xn=1) */
    /* real rate: 19231bps (52 microsecs/bit) */
    UBRR0H = 0;
    UBRR0L = 51;
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
    /* timer period: 4 microseconds = 64 machine cycles */
    OCR0A = (4*2) - 1;
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
 * flagandbit states:
 * 0: no data
 * 2: obtained '0'
 * 3: obtained '1'
 */

/*
 * sampling from the random source
 * with the timer0 COMPA ISR
 * Note: this ISR code is *interrupt driven*
 */

/* speed optimized for the ISR */
#pragma GCC optimize ("O3")

/* obtain comparator values */
#define comparator_input() (ACSR & _BV(ACO))

/*
 * declare all ISR variables
 * as volatile and mapped into AVR registers
 */
volatile register uint8_t flagandbit asm("r16");
volatile register uint8_t state asm("r15");
volatile register uint8_t oval asm("r14");

ISR(TIMER0_COMPA_vect) {
    /*
     * do nothing when flag is set
     */
    if (flagandbit != 0) {
        return;
    }
    /*
     * if flag is not set,
     * check the input state
     *
     * state: 0 -> 1st bit
     * state: 1 -> 2nd bit
     */
    if (state == 0) {
        /* save the 1st bit to oval */
        oval = comparator_input();
        /* set the 2nd bit state */
        state = 1;
    } else {
        /* obtain the 2nd bit */
        /* do nothing if two bits are the same */
        if (comparator_input() != oval) {
            /*
             * set the output if the two bits are
             * different with each other
             */
            if (oval == 0) {
                /* {1st, 2nd} = {0, 1} */
                flagandbit = 2;
            } else {
                /* {1st, 2nd} = {1, 0} */
                flagandbit = 3;
            }
        }
        /* set the 1st bit state */
        state = 0;
    }
}

/* size optimized again */
#pragma GCC optimize ("Os")

/* main function */

int main() {
    uint8_t i = 0;
    uint8_t p = 0;
    uint8_t j = 0;
    uint8_t p2 = 0;
    uint8_t p2b = 0;

    /* initialize ports, timers, serial, and IRQ */
    ioinit();

    /* initialize the global register variables */
    flagandbit = 0;
    state = 0;
    oval = 0;

    for (;;) {
        /* check whether a random bit is found */
        if (flagandbit != 0) {
            /* accumulate 8 bits */
            p = p + p;
            if ((flagandbit & 0x01) != 0) {
                /* Turn on the LED */
                PORTB |= 0x20;
                p++;
            } else {
                /* Turn off the LED */
                PORTB &= 0xdf;
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

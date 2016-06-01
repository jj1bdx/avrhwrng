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
 * for hardware randomizer connected to
 * PD7 (digital pin 7) and PD6 (digital pin 6)
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

    /* PB5 = LED on Arduino 2009/Uno */
    /* Keep it turned on, no blink */
    DDRB = 0xff;
    PORTB = 0xff;

    /* comparator disabled */
    ACSR = 0x80;

    /* USART 0 */
    /* no USART IRQ, disable TX/RX */
    UCSR0B = 0x00;
    /* clk 2x */
    UCSR0A = 0x02;
    /* async, no parity, 8bit */
    UCSR0C = 0x06;
    /* baud rate (921600bps, 16MHz, U2Xn=1) */
    /* real rate: 1000000bps (1 microsecs/bit) */
    UBRR0H = 0;
    UBRR0L = 1;
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
     * This timer defines the sampling timing rate
     * from Port D (PD7 and PD6 together)
     * The timer period should not be smaller than
     * 1000000 / 10 * 8 / 2 = 400000Hz or 2.5 microseconds
     */
    /* timer period: 40 machine cycles = 2.5 microseconds */
    OCR0A = 40 - 1;
    /* clk (16MHz or 0.0625 microsecond / count) */
    /* start timer */
    TCCR0B = 0x01;

    /* Note: interrupts are NOT enabled at all */
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

/*
 * flagandbit[01] states:
 * 0: no valid data bit
 * 1: obtained '1'
 * 2: obtained '0'
 */

/* main function */

/* raw bit test - no filtering */

int main() {
    uint8_t i = 0;
    uint8_t p = 0;

    /* initialize ports, timers, serial, and IRQ */
    ioinit();

    for (;;) {
        /* wait until OCF0A in TIFR0 enabled */
        if ((TIFR0 & 0x02) == 0) {
            continue;
        }
        /* Clear the OCF0A flag in TIFR0 */
        TIFR0 |= 0x02;
        /* put 2 bits from the top */
        p = p >> 2;
        p |= (PIND & 0xc0);
        i++;
        /* print accumulated value */
        if (i > 3) {
            putchr(p);
            /* clear counter */
            i = 0;
        }
    }
    /* NOTREACHED */
    return 0;
}

/* end of code */

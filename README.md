# Hardware Random Number Generator for Arduino

This repository contains a sample code with avr-gcc and the schematics
for a shield (extended piece of hardware) for Arduino Duemilanove / Arduino UNO.

Note: the code is *not* written in the Arduino programming framework; it
is written in C and avr-libc at <http://www.nongnu.org/avr-libc/>.

## LICENSE

* All AVR source code files are licensed by the MIT License.
* All schematics files are licensed by CC-BY-4.0.

## How it works on Version 2

Two independent noise generator circuits, amplified to the CMOS digital level,
are connected to ATmega168/328P's input pins (PD6/PD7, or Pin 6/7).

The Timer 0 is set to CTC mode with the closest larger value to the theoretical
limit, 46 machine cycles or 2.875 microseconds. The theoretical limit of the
speed is 352000bps or 2.84 microseconds, which is equal to 11000 bytes * 8
bits/byte * 2 (for von-Neumann test) * 2 (for XORing byte filter). Sampling in
an equal timing improves the quality of the obtained randomness.

The MCU runs the following program without any hardware interrupt as an infinite loop:

* Wait for the Timer 0 compare match happens, so that the sampling interval will not exceed the value given to Timer 0
* Sample the pin inputs of PD6 and PD7
* Treat the sampled two-bit pair into two independent bit streams
* Apply the von Neumann algorithm for two consecutive sampled bits *independently* for each bit stream
* Accumulate valid bits from two bit streams into single byte stream
* Filter a two-byte pair from the byte stream by XORing with each other
* Wait for finishing the USART0 transmission (serial rate: running in 111111bps for 115200bps)
* Send each filtered byte to USART0 (non-blocking)
* Go to the top and do it all over again

Random number stream is obtained through the tty device of Arduino as a binary
byte stream.

The code will run either on ATmega168 or ATmega328P. 

See `noiseshield-v2/` directory for the schematics (drawn by xcircuit). The
entire circuits can be built on a standard Arduino prototype board.

## Changes for Version 2

* 25-SEP-2015: Add Timer 0 synchronization code
* 24-SEP-2015: Initial revision, change PD7/PD6 to digital input, reduce LED blinking from each output bit to each output *byte*, remove interrupt-driven code

## How to compile

* Install avr-gcc toolchain (see <https://gist.github.com/jj1bdx/f149305a57c4cb2cef7c>)
* Compile

    # Use GNU Make (gmake on FreeBSD)
    make

## how to write the image

* Compiled files are in `compiled-files` directory

    # for ATmega168 optiboot
    avrdude -D -c arduino -p m168 -b 115200 -P /dev/cuport-name \
            -U flash:w:target-filename.hex

## Test results on Version 2

For the tagged binary `v2rev1-20150925`:

* FIPS 140-2 failure rate: ~0.0008 for ~138000 blocks
* TestU01 Rabbit and Alphabit tests passed on 64Mbyte samples
* Sampling rate: 2.875 us = ~347.8kHz 
* Output rate for v2rev1: ~10800bytes/sec = ~86.4kHz
* Transfer rate from Arduino: 115200bps, 8-bit, no parity raw bytes

## Notes

* The code is tested with avr-gcc 4.9.2 running on OS X 10.10.5.
* This program is compatible with [optiboot bootloader](https://github.com/Optiboot/optiboot/).
* PySerial required for the sample Python code. See <http://pythonhosted.org/pyserial/pyserial.html#installation>

## FYI: changes for Version 1

* 23-SEP-2015: Use -O3 altogether
* 12-AUG-2015: Canceled A0/A1 setup, no connection assumed between AIN1/D7-A1 and AIN0/D6-A0
* 23-JUL-2015: Performance values revised / *CANCELED*: assumes connection between AIN1/D7-A1 and AIN0/D6-A0 (A0/A1 set to input, no pullup)
* 19-JUL-2015: Code now targeted for ATmega168 and ATmega328p (modify Makefile: default is for ATmega168).
* 6-JUL-2015: Code tested under avr-gcc 4.9.2 with HomeBrew build environment.
* 7-MAR-2009: Code tested under AVR Studio 4.15 with WinAVR-20081205.

## FYI: how it works on Version 1

Two independent noise generator circuits are connected to ATmega168/328P's
analog comparator inputs (AIN0/AIN1, PD6/PD7, or Pin 6/7). The MCU samples the
comparator output in a fixed frequency of the Timer 0 interrupts (4
microseconds = 250kHz).

The sampled output bit stream from AVR analog comparator is filtered through
von Neumann algorithm of two adjacent sampled bits into a byte stream. Each of
the two-byte pairs of the byte stream is again filtered into one-byte stream by
XORing the bytes of the pair with each other.

The code will run either on ATmega168 or ATmega328P.

## FYI: actual output rate for Version 1

* Sampling rate: 4 us = 250kHz
* Output rate for v1rev2: ~2100bytes/sec = 17.6kHz
* Output rate for v1amp: ~3500bytes/sec = 28kHz
* Transfer rate from Arduino: 115200bps, 8-bit, no parity raw bytes

[End of README.md]

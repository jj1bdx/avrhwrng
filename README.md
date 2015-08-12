# Hardware Random Number Generator for Arduino

This repository contains a sample code with avr-gcc and the schematics
for a shield (extended piece of hardware) for Arduino Duemilanove / Arduino UNO.

Note: the code is *not* written in the Arduino programming framework; it
is written in C and avr-libc at <http://www.nongnu.org/avr-libc/>.

## Changes

* 12-AUG-2015: Canceled A0/A1 setup, no connection assumed between AIN1/D7-A1 and AIN0/D6-A0
* 23-JUL-2015: Performance values revised / *CANCELED*: assumes connection between AIN1/D7-A1 and AIN0/D6-A0 (A0/A1 set to input, no pullup)
* 19-JUL-2015: Code now targeted for ATmega168 and ATmega328p (modify Makefile: default is for ATmega168).
* 6-JUL-2015: Code tested under avr-gcc 4.9.2 with HomeBrew build environment.
* 7-MAR-2009: Code tested under AVR Studio 4.15 with WinAVR-20081205.

## How to compile

    ./build.sh

## how to write the image

    # for ATmega168 optiboot
    avrdude -D -c arduino -p m168 -b 115200 -P /dev/cuport-name \
            -U flash:w:target-filename.hex

## Actual output rate

* Sampling rate: 4 us = 250kHz
* Output rate: ~2100bytes/sec = 17.6kHz
* Transfer rate from Arduino: 115200bps, 8-bit, no parity raw bytes

## Notes

* This program can be written to Arduino with [optiboot bootloader](https://github.com/Optiboot/optiboot/).
* PySerial required. See <http://pythonhosted.org/pyserial/pyserial.html#installation>

[End of README.md]

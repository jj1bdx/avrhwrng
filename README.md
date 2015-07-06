# Hardware Random Number Generator for Arduino

This repository contains a sample code with avr-gcc and the schematics
for a shield (extended piece of hardware) for Arduino Duemilanove.

Note: the code is *not* written in the Arduino programming framework; it
is written in C and avr-libc at <http://www.nongnu.org/avr-libc/>.

## Changes

* 6-JUL-2015: Code tested under avr-gcc 4.9.2 with HomeBrew build environment.
* 7-MAR-2009: Code tested under AVR Studio 4.15 with WinAVR-20081205.

## How to compile

    ./build.sh

## Notes

* This program coexists with [optiboot bootloader](https://github.com/Optiboot/optiboot/).
* PySerial required. See <http://pythonhosted.org/pyserial/pyserial.html#installation>

[End of README.md]

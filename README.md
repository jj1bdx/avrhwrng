# Hardware Random Number Generator for Arduino

This repository contains a sample code with avr-gcc and the schematics
for a shield (extended piece of hardware) for Arduino Duemilanove / Arduino UNO.

Note: The code is solely written in C and [avr-libc](http://www.nongnu.org/avr-libc/); no Arduino framework.

## Hardware

The current circuit version is called `v2rev1`. The v2rev1 consists of two
independent noise generator circuits, and the outputs are amplified to the CMOS
digital levels, which are connected to ATmega168/328P's input pins (PD6/PD7, or
Pin 6/7).

The latest fix of `v2rev1fix1` adds a 10uF capacitor to the power supply of the noise diode circuit as a part of the voltage ripple filter, so that the noise diode circuit can accept the output of a DC-DC converter such as Maxim MAX662A.

Schematics and photos are available in `schematics/` and `photos/` directories, respectively.

## Firmware

* The current version is called `v2rev1`.
* The v2rev1 gives ~10kbytes/sec of filtered output.

### How the firmware works

The Timer 0 of ATmega168/328P is set to CTC mode with the closest larger value
to the theoretical limit, 46 machine cycles or 2.875 microseconds. The
theoretical limit of the speed is 352000bps or 2.84 microseconds, which is
equal to 11000 bytes * 8 bits/byte * 2 (for von-Neumann test) * 2 (for XORing
byte filter). Sampling in an equal timing improves the quality of the obtained
randomness.

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

The current sampling strategy of only applying von Neumann algorithm
independently for each bit stream and mixing the output into single byte stream
is chosen for reducing statistical errors.

The filtering scheme in a diagram:

```
Noise gen at PD7 --- von Neumann filter 1
                             |
             two bit streams mixed into one stream - two-byte XOR filter - output
                             |
Noise gen at PD6 --- von Neumann filter 2 
```

The code runs either on ATmega168 or ATmega328P (only a vector address
difference).  Tested both on Arduino Duemilanove and Arduino UNO R3 boards.

## Documentation

* See `presentations/ipsj-iots2015/` for the set of the documentation (PDF paper in Japanese, slides in English) of my presentation at IPSJ IOTS2015 Symposium on 27-NOV-2015.
* 日本語での解説は技術評論社Software Design誌2016年9月号pp. 96-101「乱数を使いこなす(2):物理乱数ハードウェアを作る」をご参照ください

## How to compile

* Install avr-gcc toolchain (see <https://gist.github.com/jj1bdx/f149305a57c4cb2cef7c>)
* Compile

```
# Use GNU Make (gmake on FreeBSD)
make
```

## how to write the image

Compiled files are in `compiled-files` directory.

For loading into an optiboot-ready device, use:

```
# for ATmega168 optiboot
# (for ATmega328p, change m168 to m328p)
avrdude -D -c arduino -p m168 -b 115200 -P /dev/cuport-name \
        -U flash:w:target-filename.hex
```

You need to configure the following fuse bits for a production image:

* Set BODLEVEL to 2.7V (only BODLEVEL1 is programmed to '0')
* Disable SPIEN (to prevent external programming)
* Disable BOOTRST (boot from $0000)
* Set the lock bit

Use the following command sequence for AVR Dragon with ATmega168:

```
# for ATmega168 AVR Dragon
avrdude -D -v -p m168 -c dragon_pp -P usb \
        -e -u -U lock:w:0xff:m \
        -U efuse:w:0xff:m -U hfuse:w:0xfd:m -U lfuse:w:0xff:m \
        -U flash:w:avrhwrng-v2rev1-ATmega168-20150925.hex \
        -U lock:w:0xef:m
```

Use the following command sequence for AVR Dragon with ATmega328p (*the fuse
bits are different from those of ATmega168)*:

```
# for ATmega328p AVR Dragon
# NOTE WELL: the fuse bits are different from those of ATmega168!
avrdude -D -v -p m328p -c dragon_pp -P usb \
        -e -u -U lock:w:0xff:m \
        -U efuse:w:0xfd:m -U hfuse:w:0xff:m -U lfuse:w:0xff:m \
        -U flash:w:avrhwrng-v2rev1-ATmega328p-20150925.hex \
        -U lock:w:0xef:m
```

## Test results on Version 2

For the tagged binary `v2rev1-20150925`:

* FIPS 140-2 failure rate: ~0.0008 (an example: 173 for 214992 blocks, tested by rngtest)
* TestU01 Rabbit and Alphabit tests passed in most cases (~95%) on 1Mbit
  samples; 1432 failures detected out of 28700 tests (1 Rabbit, 6
  Alphabit tests for each sample) performed on 4100 samples
* Sampling rate: 2.875 us = ~347.8kHz 
* Output rate for v2rev1: ~10800bytes/sec = ~86.4kHz
* Transfer rate from Arduino: 115200bps, 8-bit, no parity raw bytes

## Notes

* Compilation tested on: avr-gcc 4.9.2, OS X 10.10.5
* Compilation tested on: avr-gcc 7.1.0, macOS 10.12.6
* Different compilers may generate different binaries.
* This program is compatible with [optiboot bootloader](https://github.com/Optiboot/optiboot/).

## Related tools

* FreeBSD driver for random(4) and the feeder: <https://github.com/jj1bdx/freebsd-dev-trng>
* OS X feeder: <https://github.com/jj1bdx/osx-devrandom-feeder>
* Arduino's ATmega16u2 USB firmware (for reassignment of USB VID/PID): <https://github.com/jj1bdx/arduino-atmega16u2>
* FreeBSD RNG test tool: <https://github.com/jj1bdx/rngtest>

## LICENSE

* All AVR source code files are licensed by the MIT License.
* All schematics files are licensed by CC-BY-4.0.

[End of README.md]

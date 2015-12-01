# Hardware Random Number Generator for Arduino

This repository contains a sample code with avr-gcc and the schematics
for a shield (extended piece of hardware) for Arduino Duemilanove / Arduino UNO.

Note: the code is *not* written in the Arduino programming framework; it
is written in C and avr-libc at <http://www.nongnu.org/avr-libc/>.

## LICENSE

* All AVR source code files are licensed by the MIT License.
* All schematics files are licensed by CC-BY-4.0.

## Documentation

* See `ipsj-iots2015/` for the set of the documentation (PDF paper in Japanese,
  slides in English) of my presentation at IPSJ IOTS2015 Symposium on 27-NOV-2015.

## Preliminary test on the rawtest code

26-OCT-2015: removal of filtering from the noise generation circuits conducted.

By running `avrhwrng-rawtest.c`, all the mixing/filtering code have been
*removed*, and the AVR outputs random number bytes are just filled in by the
sampled raw two-bit sequence of PD7 and PD6. No other alteration. 

The test was conducted on an Arduino Uno R3 with avrhwrng v2rev1 board. The
sampling rate was 400kHz, and the output rate was 1Mbps. The actual output rate
measured was ~76kbytes/sec.

Obviously the output will not pass the statictic tests due to possible skew and
bias.  Filtering this by SHA512 as a function defined as 

```
[512 bytes raw input] + [32 byte output from the previous SHA512 output] -> [64
bytes of SHA512 output as a part of random number stream]
```

made the output perfectly usable as a well-whitened random number sequence.

The FIPS 140-2 rngtest result for 1597357500 bytes of output resulted as
follows (summary: test failure rate: 0.079%):

```
~/src/rngtest/rngtest -c 638943 < poi1filt.bin
rngtest 2-unofficial-mt.14
Copyright (c) 2004 by Henrique de Moraes Holschuh
This is free software; see the source for copying conditions.  There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

rngtest: starting FIPS tests...
rngtest: bits received from input: 12778860032
rngtest: FIPS 140-2 successes: 638440
rngtest: FIPS 140-2 failures: 503
rngtest: FIPS 140-2(2001-10-10) Monobit: 62
rngtest: FIPS 140-2(2001-10-10) Poker: 74
rngtest: FIPS 140-2(2001-10-10) Runs: 188
rngtest: FIPS 140-2(2001-10-10) Long run: 180
rngtest: FIPS 140-2(2001-10-10) Continuous run: 0
rngtest: input channel speed: (min=762.939; avg=12736.675; max=19073.486)Mibits/s
rngtest: FIPS tests speed: (min=35.453; avg=126.549; max=129.752)Mibits/s
rngtest: Program run time: 97340320 microseconds
```

TestU01 Rabbit and Alphabit tests passed in most cases (~94.8%) on 1Mbit
samples; 1180 failures detected out of 22750 tests (1 Rabbit, 6 Alphabit tests
for each sample) performed on 3250 samples.  The longer bit sample tests of
TestU01 Rabbit and Alphabit tests for the 1992Mbit (249Mbyte) sample were all
passed.

Result updated 11-MOV-2015: a dieharder result of the 2179368064 byte output
was very good, only the following three tests showed WEAK, no FAILED test (note
that the input sequence was rewound as dieharder demanded):

```
    diehard_rank_6x8|   0|    100000|     100|0.99942491|   WEAK
        diehard_sums|   0|       100|     100|0.00071977|   WEAK
      rgb_lagged_sum|  31|   1000000|     100|0.00057416|   WEAK
```

The result poses a fundamental question: *filtering on the hardware RNG board
is utterly meaningless if a sufficient whitening or
statictically/cryptographically strong bit mixing is performed on the host
side.*  Further investigation needed.

## How it works on Version 2

Two independent noise generator circuits, amplified to the CMOS digital levels,
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

The code runs either on ATmega168 or ATmega328P (only a vector address
difference).  Tested both on Arduino Duemilanove and Arduino UNO R3 boards.

See `noiseshield-v2/` directory for the schematics (drawn by xcircuit). The
entire circuits can be built on a standard Arduino prototype board.

## On stream mixing strategy

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

Treating the two bit streams of PD7 and PD6 as two independent XORing byte
streams caused the following TestU01 Rabbit test errors:

```
  1  MultinomialBitsOver              eps
  2  ClosePairsBitMatch, t = 2      1.9e-68
  3  ClosePairsBitMatch, t = 4     1.0e-146
```

## Changes for Version 2

* 25-SEP-2015: Add Timer 0 synchronization code
* 24-SEP-2015: Initial revision, change PD7/PD6 to digital input, reduce LED blinking from each output bit to each output *byte*, remove interrupt-driven code

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

* The code is compiled with avr-gcc 4.9.2 running on OS X 10.10.5. Different compilers may generate different binaries.
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

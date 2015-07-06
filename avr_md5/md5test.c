/*
 * ATmega 328 (and compatible) MD5 hash function (RFC 1321) optimized assembly
 * implementation.
 *
 * by Mateusz "j00ru" Jurczyk
 * http://j00ru.vexillium.org/
 *
 * -- License
 *
 * Copyright (C) 2012 by Mateusz Jurczyk
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
 *
 * -- Usage
 *
 * The file implements the core computational md5_transform procedure. Together
 * with the MD5 interface (md5.c), it can be linked with any ATmega328 (or
 * potentially other AVR MCUs) C/C++ code and used for fast MD5 computation.
 * 
 * Compilation:
 *   avr-gcc -Os -DF_CPU=16000000UL -mmcu=atmega328p -c -o md5.o md5.c md5.S
 *   avr-gcc -mmcu=atmega328p -o md5test md5.o md5test.c
 *
 * Usage with simulavr and avr-gdb (the following checks exit code to see if
 * all tests passed):
 *
 *   (gdb) file md5test
 *   Reading symbols from md5test...(no debugging symbols found)...done.
 *   (gdb) target remote localhost:1212
 *   Remote debugging using localhost:1212
 *   warning: Can not parse XML target description; XML support was disabled at compi
 *   le time
 *   0x00000000 in __vectors ()
 *   (gdb) load
 *   Loading section .text, size 0x1fbe lma 0x0
 *   Loading section .data, size 0x1b8 lma 0x1fbe
 *   Start address 0x0, load size 8566
 *   Transfer rate: 269 KB/sec, 856 bytes/write.
 *   (gdb) b exit
 *   Breakpoint 1 at 0x1fba
 *   (gdb) c
 *   Continuing.
 *   [New Thread 1]
 *
 *   Breakpoint 1, 0x00001fba in exit ()
 *   (gdb) info reg r24
 *   r24            0x0      0
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "md5.h"

struct _md5_test {
  uint8_t *data;
  size_t length;
  uint32_t hash[4];
} md5_tests[] = {
  {"", 0,
  {0xd98c1dd4, 0x04b2008f, 0x980980e9, 0x7e42f8ec}},
  {"asdf", 4,
  {0x03c82e91, 0xe449ceb2, 0x8d0641a5, 0x70b55a49}},
  {"testtest", 8,
  {0xc671a605, 0x12eaef6a, 0x768bc04c, 0xbb306dea}},
  {"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 40,
  {0x541e45a9, 0xade43a4b, 0x28d2aa6b, 0x9861a4d5}},
  {"http://j00ru.vexillium.org/", 27,
  {0x1cfb6cfe, 0xe480797c, 0x1dcf065b, 0xade45864}},
  {"0x410x410x410x410x410x410x410x410x410x410x410x410x410x410x410x41", 64,
  {0x83d41a43, 0x8d71392d, 0xd56b8fe8, 0x2c73c17f}},
  {"Lorem ipsum dolor sit amet, consectetur adipisicing eli", 56,
  {0x8003b901, 0x1908334f, 0x8d27182b, 0xb1fb3662}},
  {"Lorem ipsum dolor sit amet, consectetur adipisicing el", 55,
  {0xb8fbfe3e, 0x724b5b69, 0x132d9d10, 0xfdf620a4}},
  {NULL,   0,
  {0x00000000, 0x00000000, 0x00000000, 0x00000000}},
};

int main(int argc, char **argv) {
  unsigned int i;
  uint8_t hash[16];

  for (i = 0; md5_tests[i].data != NULL; i++) {
    md5(md5_tests[i].data, md5_tests[i].length, hash);
    if (memcmp(hash, md5_tests[i].hash, 16)) {
      exit(1);
    }
  }

  exit(0);
}


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
 */
#ifndef _AVR_MD5_H_
#define _AVR_MD5_H_

#include <stdint.h>

#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define kMD5BlockBits   512
#define kMD5StateBits   128
#define kMD5BlockBytes  (kMD5BlockBits / 8)
#define kMD5OutputBytes (kMD5StateBits / 8)

/* MD5 state */
typedef struct md5_ctx {
  uint32_t h[kMD5OutputBytes];

  uint8_t buffer[kMD5BlockBytes];
  size_t length;
  
  uint64_t msg_length;
} md5_ctx_t;

/* public interface */
void md5_init(md5_ctx_t *c);
void md5_update(md5_ctx_t *c, const void *block, size_t length);
void md5_finalize(md5_ctx_t *c, uint8_t hash[16]);
void md5(const void *data, size_t length, uint8_t hash[16]);

/* implemented in md5.S, used internally */
void md5_transform(uint32_t state[4], const void *block);

#endif  // _AVR_MD5_H_


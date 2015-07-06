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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "md5.h"

void md5_init(md5_ctx_t *c) {
  c->h[0] = 0x67452301;
  c->h[1] = 0xefcdab89;
  c->h[2] = 0x98badcfe;
  c->h[3] = 0x10325476;
  
  c->length = 0;
  c->msg_length = 0;
}

void md5_update(md5_ctx_t *c, const void *block, size_t length) {
  /* Update message length. It is represented as bit count at the end
     of the stream, thus the multiplication by 8. */
  c->msg_length += (length << 3);

  /* Fill the internal buffer with incoming data, if it already is not empty */
  if (c->length > 0) {
    size_t pad_size = MIN(kMD5BlockBytes - c->length, length);
    memcpy(&c->buffer[c->length], block, pad_size);
    
    length -= pad_size;
    block += pad_size;
    c->length += pad_size;

    if (c->length == kMD5BlockBytes) {
      md5_transform(c->h, c->buffer);
      c->length = 0;
    }
  }

  /* Perform transformations on all available 512-bit blocks */
  while (length >= kMD5BlockBytes) {
    md5_transform(c->h, block);
    block += kMD5BlockBytes;
    length -= kMD5BlockBytes;
  }

  /* Store the remaining part in internal buffer */
  if (length > 0) {
    memcpy(c->buffer, block, length);
    c->length = length;
  }
}

void md5_finalize(md5_ctx_t *c, uint8_t hash[kMD5OutputBytes]) {
  /* Add trailing bit */
  c->buffer[c->length++] = 0x80;
  if (c->length == kMD5BlockBytes) {
    md5_transform(c->h, c->buffer);
    c->length = 0;
  }

  /* Pad with zeros if current buffer is longer than 448 bits */
  if (c->length > kMD5BlockBytes - sizeof(uint64_t)) {
    memset(&c->buffer[c->length], 0, kMD5BlockBytes - c->length),
    md5_transform(c->h, c->buffer);
    c->length = 0;
  }
  
  /* Perform the final transformation on last block */
  memset(&c->buffer[c->length], 0, kMD5BlockBytes - sizeof(uint64_t) - c->length);
  memcpy(&c->buffer[kMD5BlockBytes - sizeof(uint64_t)], &c->msg_length, sizeof(uint64_t));
  md5_transform(c->h, c->buffer);

  /* Copy the final hash over to output buffer */
  memcpy(hash, c->h, kMD5OutputBytes);
}

void md5(const void *data, size_t length, uint8_t hash[kMD5OutputBytes]) {
  static md5_ctx_t ctx;

  md5_init(&ctx);
  md5_update(&ctx, data, length);
  md5_finalize(&ctx, hash);
}


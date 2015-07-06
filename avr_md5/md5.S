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

/*
 * Storage usage:
 *   Bytes  Location          Description
 *       4   r5: r4: r3: r2   MD5 state variable A
 *       4   r9: r8: r7: r6   MD5 state variable B
 *       4  r13:r12:r11:r10   MD5 state variable C
 *       4  r17:r16:r15:r14   MD5 state variable D
 *       4  r21:r20:r19:r18   Temporary for calculation per round
 *       4  r25:r24:r23:r22   Temporary for calculation per round
 *       4          r27:r26   Temporary for calculation per round
 *       2          r29:r28   Input data pointer
 *       2          r31:r30   MD5 state pointer
 */
__w__     = 2  /* MD5 state variable A */
__x__     = 6  /* MD5 state variable B */
__y__     = 10 /* MD5 state variable C */
__z__     = 14 /* MD5 state variable D */
__tmp1__  = 18 /* Temporary */
__tmp2__  = 22 /* Temporary */
__tmp3__  = 26 /* Temporary */


/* 32-bit register move.
   r(a+3 .. a) = r(b+3 .. b) */
.macro _MOVR a b
  movw  (\a + 0), (\b + 0)
  movw  (\a + 2), (\b + 2)
.endm

/* 32-bit indexed memory to register load.
   r(a+3 .. a) = sram(Y+q*4+3 .. Y+q*4) */
.macro _LDD a q
  ldd   (\a + 0), Y + (\q * 4)
  ldd   (\a + 1), Y + (\q * 4) + 1
  ldd   (\a + 2), Y + (\q * 4) + 2
  ldd   (\a + 3), Y + (\q * 4) + 3
.endm

/* 32-bit incremented memory load.
   r(a+3 .. a) = sram(Z+3 .. Z)
   Z = Z + 4 */
.macro _LDZ a
  ld    (\a + 0), Z+
  ld    (\a + 1), Z+
  ld    (\a + 2), Z+
  ld    (\a + 3), Z+
.endm

/* 32-bit incremented memory store.
   sram(Z+3 .. Z) = r(a+3 .. a)
   Z += 4 */
.macro _STZ a
  st    Z+, (\a + 0)
  st    Z+, (\a + 1)
  st    Z+, (\a + 2)
  st    Z+, (\a + 3)
.endm

/* 32-bit register addition.
   r(a+3 .. a) += r(b+3 .. b) */
.macro _ADDR a b
  add   (\a + 0), (\b + 0)
  adc   (\a + 1), (\b + 1)
  adc   (\a + 2), (\b + 2)
  adc   (\a + 3), (\b + 3)
.endm

/* 32-bit immediate to register addition.
   r(a+3 .. a) += b */
.macro _ADDI a b
  subi  (\a + 0),  -(\b >> 0)       & 0xff
  sbci  (\a + 1), (-(\b >> 8)  - 1) & 0xff 
  sbci  (\a + 2), (-(\b >> 16) - 1) & 0xff
  sbci  (\a + 3), (-(\b >> 24) - 1) & 0xff
.endm

/* 32-bit register and.
   r(a+3 .. a) &= r(b+3 .. b) */
.macro _AND a b
  and   (\a + 0), (\b + 0)
  and   (\a + 1), (\b + 1)
  and   (\a + 2), (\b + 2)
  and   (\a + 3), (\b + 3)
.endm

/* 32-bit register not (one's complement).
   r(a+3 .. a) = ~r(a+3 .. a) */
.macro _NOT a
  com   (\a + 0)
  com   (\a + 1)
  com   (\a + 2)
  com   (\a + 3)
.endm

/* 32-bit register or.
   r(a+3 .. a) |= r(b+3 .. b) */
.macro _OR a b
  or    (\a + 0), (\b + 0)
  or    (\a + 1), (\b + 1)
  or    (\a + 2), (\b + 2)
  or    (\a + 3), (\b + 3)
.endm

/* Optimized 32-bit register rotation with constant shift.
   ROTATE_LEFT(r(a+3 .. a), b) */
.macro _ROL a b
 /* Rotation by 16 is implemented in word granularity.
    swap(r(a+3 .. a+2), r(a+1 .. a)) */
.if \b >= 16
  movw  __tmp1__, (\a + 0)
  movw  (\a + 0), (\a + 2)
  movw  (\a + 2), __tmp1__
.endif
  /* Rotation by 8 is implemented in byte granularity.
     r(a+3) = r(a+2)
            .
            .
     r(a+0) = r(a+3) */
.if (\b & 15) >= 8
  mov   r0, (\a + 3)
  mov   (\a + 3), (\a + 2)
  mov   (\a + 2), (\a + 1)
  mov   (\a + 1), (\a + 0)
  mov   (\a + 0), r0
.endif
  /* If there are at least 5 rotations left, we can perform them all at once by
   * using multiplication instruction (mul) with (1<<x) operands. The following
   * assembly always takes 20 cycles to execute:
   *  + 1  cycle to initialize __tmp3__ operand.
   *  + 16 cycles for transformations, four [mul(2), mov(1), or(1)] for each
   *  byte.
   *  + 2  cycles to save the result back to destination register.
   *  + 1  cycle to zero out r1.
   *
   * Since performing rotations one-by-one takes 5 cycles per rotation, we are
   * able to save up to 15 cycles by using the formula below.
   * 
   * See ATmega328 datasheet for instruction clock counts:
   * http://www.atmel.com/Images/8271s.pdf
   */
.if (\b & 7) >= 5
  ldi   __tmp3__, (1 << (\b & 7))
  mul   (\a + 0), __tmp3__
  mov   (__tmp1__ + 0), r0
  mov   (__tmp1__ + 1), r1
  mul   (\a + 1), __tmp3__
  or    (__tmp1__ + 1), r0
  mov   (__tmp1__ + 2), r1
  mul   (\a + 2), __tmp3__
  or    (__tmp1__ + 2), r0
  mov   (__tmp1__ + 3), r1
  mul   (\a + 3), __tmp3__
  or    (__tmp1__ + 3), r0
  or    (__tmp1__ + 0), r1
  movw  (\a + 0), (__tmp1__ + 0)
  movw  (\a + 2), (__tmp1__ + 2)
  clr   r1
.else
  /* Up to 4 remaining rotations are performed singly. */
  .if (\b & 7) >= 4
    lsl   (\a + 0)
    rol   (\a + 1)
    rol   (\a + 2)
    rol   (\a + 3)
    adc   (\a + 0), r1
  .endif
  .if (\b & 7) >= 3
    lsl   (\a + 0)
    rol   (\a + 1)
    rol   (\a + 2)
    rol   (\a + 3)
    adc   (\a + 0), r1
  .endif
  .if (\b & 7) >= 2
    lsl   (\a + 0)
    rol   (\a + 1)
    rol   (\a + 2)
    rol   (\a + 3)
    adc   (\a + 0), r1
  .endif
  .if (\b & 7) >= 1
    lsl   (\a + 0)
    rol   (\a + 1)
    rol   (\a + 2)
    rol   (\a + 3)
    adc   (\a + 0), r1
  .endif
.endif
.endm

/* 32-bit register xor.
   r(a+3 .. a) ^= r(b+3 .. b) */
.macro _XOR a b
  eor   (\a + 0), (\b + 0)
  eor   (\a + 1), (\b + 1)
  eor   (\a + 2), (\b + 2)
  eor   (\a + 3), (\b + 3)
.endm

/* MD5 helper macro for Round 0 */
.macro FF a b c d k s t
  _MOVR __tmp1__, \c
  _XOR  __tmp1__, \d
  _AND  __tmp1__, \b
  _XOR  __tmp1__, \d
  TAIL \a, \b, \k, \s, \t
.endm

/* MD5 helper macro for Round 1 */
.macro GG a b c d k s t
  _MOVR __tmp1__, \c
  _XOR  __tmp1__, \b
  _AND  __tmp1__, \d
  _XOR  __tmp1__, \c
  TAIL \a, \b, \k, \s, \t
.endm

/* MD5 helper macro for Round 2 */
.macro HH a b c d k s t
  _MOVR __tmp1__, \c
  _XOR  __tmp1__, \d
  _XOR  __tmp1__, \b
  TAIL \a, \b, \k, \s, \t
.endm

/* MD5 helper macro for Round 3 */
.macro II a b c d k s t
  _MOVR __tmp1__, \d
  _NOT  __tmp1__
  _OR   __tmp1__, \b
  _XOR  __tmp1__, \c
  TAIL \a, \b, \k, \s, \t
.endm

/* Common tail for MD5 transformations */
.macro TAIL a b k s t
  _LDD  __tmp2__, \k
  _ADDR __tmp2__, __tmp1__
  _ADDI __tmp2__, \t
  _ADDR __tmp2__, \a
  _ROL  __tmp2__, \s
  _ADDR __tmp2__, \b
  _MOVR \a, __tmp2__
.endm

	.text
/*
 * void md5_transform(uint32_t state[4], const void *block);
 */
.global	md5_transform
	.type	md5_transform, @function
md5_transform:

  /* Preserve non-volatile registers */
	push r2
	push r3
	push r4
	push r5
	push r6
	push r7
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push r16
	push r17
	push r29
	push r28

  movw r28, r22 /* r29:r28 (Y) input data */
  movw r30, r24 /* r31:r30 (Z) pointer to state */
  /* Load MD5 state */
  _LDZ __w__
  _LDZ __x__
  _LDZ __y__
  _LDZ __z__
  
  /* First round */
	FF __w__, __x__, __y__, __z__,  0,  7, 0xD76AA478
	FF __z__, __w__, __x__, __y__,  1, 12, 0xE8C7B756
	FF __y__, __z__, __w__, __x__,  2, 17, 0x242070DB
	FF __x__, __y__, __z__, __w__,  3, 22, 0xC1BDCEEE
	FF __w__, __x__, __y__, __z__,  4,  7, 0xF57C0FAF
	FF __z__, __w__, __x__, __y__,  5, 12, 0x4787C62A
	FF __y__, __z__, __w__, __x__,  6, 17, 0xA8304613
	FF __x__, __y__, __z__, __w__,  7, 22, 0xFD469501
	FF __w__, __x__, __y__, __z__,  8,  7, 0x698098D8
	FF __z__, __w__, __x__, __y__,  9, 12, 0x8B44F7AF
	FF __y__, __z__, __w__, __x__, 10, 17, 0xFFFF5BB1
	FF __x__, __y__, __z__, __w__, 11, 22, 0x895CD7BE
	FF __w__, __x__, __y__, __z__, 12,  7, 0x6B901122
	FF __z__, __w__, __x__, __y__, 13, 12, 0xFD987193
	FF __y__, __z__, __w__, __x__, 14, 17, 0xA679438E
	FF __x__, __y__, __z__, __w__, 15, 22, 0x49B40821

  /* Second round */
	GG __w__, __x__, __y__, __z__,  1,  5, 0xF61E2562
	GG __z__, __w__, __x__, __y__,  6,  9, 0xC040B340
	GG __y__, __z__, __w__, __x__, 11, 14, 0x265E5A51
	GG __x__, __y__, __z__, __w__,  0, 20, 0xE9B6C7AA
	GG __w__, __x__, __y__, __z__,  5,  5, 0xD62F105D
	GG __z__, __w__, __x__, __y__, 10,  9, 0x02441453
	GG __y__, __z__, __w__, __x__, 15, 14, 0xD8A1E681
	GG __x__, __y__, __z__, __w__,  4, 20, 0xE7D3FBC8
	GG __w__, __x__, __y__, __z__,  9,  5, 0x21E1CDE6
	GG __z__, __w__, __x__, __y__, 14,  9, 0xC33707D6
	GG __y__, __z__, __w__, __x__,  3, 14, 0xF4D50D87
	GG __x__, __y__, __z__, __w__,  8, 20, 0x455A14ED
	GG __w__, __x__, __y__, __z__, 13,  5, 0xA9E3E905
	GG __z__, __w__, __x__, __y__,  2,  9, 0xFCEFA3F8
	GG __y__, __z__, __w__, __x__,  7, 14, 0x676F02D9
	GG __x__, __y__, __z__, __w__, 12, 20, 0x8D2A4C8A

  /* Third round */
	HH __w__, __x__, __y__, __z__,  5,  4, 0xFFFA3942
	HH __z__, __w__, __x__, __y__,  8, 11, 0x8771F681
	HH __y__, __z__, __w__, __x__, 11, 16, 0x6D9D6122
	HH __x__, __y__, __z__, __w__, 14, 23, 0xFDE5380C
	HH __w__, __x__, __y__, __z__,  1,  4, 0xA4BEEA44
	HH __z__, __w__, __x__, __y__,  4, 11, 0x4BDECFA9
	HH __y__, __z__, __w__, __x__,  7, 16, 0xF6BB4B60
	HH __x__, __y__, __z__, __w__, 10, 23, 0xBEBFBC70
	HH __w__, __x__, __y__, __z__, 13,  4, 0x289B7EC6
	HH __z__, __w__, __x__, __y__,  0, 11, 0xEAA127FA
	HH __y__, __z__, __w__, __x__,  3, 16, 0xD4EF3085
	HH __x__, __y__, __z__, __w__,  6, 23, 0x04881D05
	HH __w__, __x__, __y__, __z__,  9,  4, 0xD9D4D039
	HH __z__, __w__, __x__, __y__, 12, 11, 0xE6DB99E5
	HH __y__, __z__, __w__, __x__, 15, 16, 0x1FA27CF8
	HH __x__, __y__, __z__, __w__,  2, 23, 0xC4AC5665

  /* Fourth round */
	II __w__, __x__, __y__, __z__,  0,  6, 0xF4292244
	II __z__, __w__, __x__, __y__,  7, 10, 0x432AFF97
	II __y__, __z__, __w__, __x__, 14, 15, 0xAB9423A7
	II __x__, __y__, __z__, __w__,  5, 21, 0xFC93A039
	II __w__, __x__, __y__, __z__, 12,  6, 0x655B59C3
	II __z__, __w__, __x__, __y__,  3, 10, 0x8F0CCC92
	II __y__, __z__, __w__, __x__, 10, 15, 0xFFEFF47D
	II __x__, __y__, __z__, __w__,  1, 21, 0x85845DD1
	II __w__, __x__, __y__, __z__,  8,  6, 0x6FA87E4F
	II __z__, __w__, __x__, __y__, 15, 10, 0xFE2CE6E0
	II __y__, __z__, __w__, __x__,  6, 15, 0xA3014314
	II __x__, __y__, __z__, __w__, 13, 21, 0x4E0811A1
	II __w__, __x__, __y__, __z__,  4,  6, 0xF7537E82
	II __z__, __w__, __x__, __y__, 11, 10, 0xBD3AF235
	II __y__, __z__, __w__, __x__,  2, 15, 0x2AD7D2BB
	II __x__, __y__, __z__, __w__,  9, 21, 0xEB86D391

  /* md5[0] += h[0]
     md5[1] += h[1]
     md5[2] += h[2]
     md5[3] += h[3] */
  sbiw r30, 16
  _LDZ __tmp1__
  _ADDR __w__, __tmp1__
  _LDZ __tmp1__
  _ADDR __x__, __tmp1__
  _LDZ __tmp1__
  _ADDR __y__, __tmp1__
  _LDZ __tmp1__
  _ADDR __z__, __tmp1__

  /* Save state to output buffer */
  sbiw r30, 16
  _STZ __w__
  _STZ __x__
  _STZ __y__
  _STZ __z__

  /* Restore non-volatile registers */
	pop r28
	pop r29
	pop r17
	pop r16
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop r7
	pop r6
	pop r5
	pop r4
	pop r3
	pop r2
	ret

	.size	md5_transform, .-md5_transform


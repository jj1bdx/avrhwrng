/*
 * Testing Hardware RNG output
 * by TestU01 Rabbit and Alphabit tests
 *
 * To compile (on FreeBSD Port math/testu01):
 * cc -O3 -o t-xor t-xor.c -I/usr/local/include/TestU01 -L/usr/local/lib -ltestu01 -lprobdist -lmylib -lm
 */

#include "gdef.h"
#include "swrite.h"
#include "bbattery.h"

/* 2^29 */
#define BITSIZE (double)(536870912)

int main (void)
{
    swrite_Basic = FALSE;
    bbattery_RabbitFile ("testxor2", BITSIZE);
    bbattery_BlockAlphabitFile ("testxor2", BITSIZE);
    return 0;
}

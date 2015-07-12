# AVR Hardware RNG test code and results

* TestU01 Rabbit and Alphabit tests for 2^29 bits of output
* _NOTE: no SmallCrush, Crush, or BigCrush test conducted_ (output size too small)
* Code run on FreeBSD 10.2-PRERELEASE
* See <http://www.iro.umontreal.ca/~simardr/testu01/guideshorttestu01.pdf> for the further details

## Files

* testxor2: the binary output (slightly larger than 2^26 bytes)
* t-xor.c: C test code with TestU01 library
* t-xor.result.txt: test result for testxor2


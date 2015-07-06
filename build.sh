#!/bin/sh
make -f ./Makefile.byte $*
make -f ./Makefile.hexdump $*
make -f ./Makefile.raw $*

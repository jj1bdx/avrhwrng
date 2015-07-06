#!/usr/bin/env python

import sys, serial

if __name__ == '__main__':
    ser = serial.Serial('/dev/cu.usbserial-A8004ISG', 115200, timeout=10, xonxoff=0, rtscts=0)
    # ser.open()
    bb = bytearray(512)
    while 1:
        ba = bytearray(ser.read(1024))
        for i in range(512):
            j = i * 2
            bb[i] = ba[j] ^ ba[j+1]
	sys.stdout.write(bb)
        sys.stdout.flush()

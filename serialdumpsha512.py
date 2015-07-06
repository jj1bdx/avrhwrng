#!/usr/bin/env python

import sys
import serial
import hashlib

if __name__ == '__main__':
    m = hashlib.sha512()
    ser = serial.Serial('/dev/cu.usbserial-A8004ISG', 115200, timeout=10, xonxoff=0, rtscts=0)
    # ser.open()
    o = b""
    while 1:
	m.update(ser.read(1024) + o)
        o = m.digest()
        sys.stdout.write(o)
        sys.stdout.flush()

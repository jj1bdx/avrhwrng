#!/usr/bin/env python

import sys, serial

if __name__ == '__main__':
    ser = serial.Serial(
            port='/dev/cu.usbserial-A8004ISG',
            baudrate=38400,
            timeout=10,
            xonxoff=0,
            rtscts=0)
    # ser.open()
    # discard first 128 bytes
    dummy = ser.read(128)
    # receive loop
    while 1:
	s = ser.read(64)
	sys.stdout.write(s)
        sys.stdout.flush()

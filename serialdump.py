#!/usr/bin/env python

import sys, serial

if __name__ == '__main__':
    ser = serial.Serial('/dev/cu.usbserial-A8004ISG', 115200, timeout=10, xonxoff=0, rtscts=0)
    # ser.open()
    while 1:
	s = ser.read(1024)
	sys.stdout.write(s)

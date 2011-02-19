#!/usr/local/bin/python

import serial
import hashlib

if __name__ == '__main__':
    m = hashlib.sha256()
    rand = open('/dev/random', 'r+')
    ser = serial.Serial('/dev/cuaU0', 115200, timeout=5, xonxoff=0, rtscts=0)
    ser.open()
    while 1:
	rand.write(ser.read(512))
	# m.update(ser.read(512))
        # rand.write(m.digest())

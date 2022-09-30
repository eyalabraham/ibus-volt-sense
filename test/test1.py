#!/usr/bin/python
#####################################################################
#
# test1.py
#
#   Very basic tester that listens to the data sent by the receiver
#   on the bus and displays the contact with a time stamp every four bytes.
#   This tester depends on connecting the receiver to the PC through
#   a FTDI-like connection converting the receiver's serial output
#   to a USB serial device/port. See README.md for connection.
#
#####################################################################

import serial
import time

#ser = serial.Serial('/dev/myTTY', baudrate=115200, write_timeout=0.5, timeout=0.5)
ser = serial.Serial('/dev/ttyUSB0', baudrate=115200, write_timeout=0.5, timeout=0.5)
print(ser.name, ser.baudrate, ser.bytesize, ser.parity, ser.stopbits)
#ser.write(b'send')
now = time.time()
while True:
    resp = ser.read(4)
    print(time.time()-now)
    now = time.time()
    for byte in resp:
        print(hex(byte))

ser.close()

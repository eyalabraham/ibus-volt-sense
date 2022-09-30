#!/usr/bin/python
#####################################################################
#
# test2.py
#
#   Listens to the data sent by the receiver on the bus,
#   parses the packet's data and displays the result.
#   This tester depends on connecting the receiver to the PC through
#   a FTDI-like connection converting the receiver's serial output
#   to a USB serial device/port. See README.md for connection.
#
#####################################################################

import serial
import time

def get_packet(serial_in):
    '''
    Get a RC receiver data packet from the serial link.
    Returns packet length as integer and byte list packet data
    without packet length byte.
    '''
    packet_length = int.from_bytes(serial_in.read(1),'little')
    resp = serial_in.read((packet_length)-1)
    
    return (packet_length, resp)
    
def parse_packet(length, data):
    '''
    Parse RC receiver packet and return command, sensor ID and validity of checksum
    '''
    # Calculate checksum
    checksum = length
    for byte in range(length-3):
        checksum = checksum + data[byte]
        
    checksum = 65535 - checksum
    
    # Check checksum validity
    packet_checksum = int.from_bytes(data[-2:],'little')
    if packet_checksum != checksum:
        valid_checksum = False
    else:
        valid_checksum = True
        
    # Command byte
    cmd_sensor = data[0]
    command = cmd_sensor >> 4
    sensor = cmd_sensor & 15
    
    return command, sensor, valid_checksum

#ser = serial.Serial('/dev/myTTY', baudrate=115200, write_timeout=0.5, timeout=0.5)
ser = serial.Serial('/dev/ttyUSB0', baudrate=115200, write_timeout=0.5, timeout=0.5)
print(ser.name, ser.baudrate, ser.bytesize, ser.parity, ser.stopbits)
#ser.write(b'send')
now = time.time()
while True:
    length, data = get_packet(ser)
    command, sensor, checksum_ok = parse_packet(length, data)

    print(length, command, sensor, checksum_ok)

    print(time.time()-now)
    now = time.time()

ser.close()



#!/usr/bin/python
#####################################################################
#
# test3.py
#
#   Listens to the data sent by the receiver on the bus,
#   parses the packet's data and respond to receiver's commands.
#   The test program responds to sensor #1 as the external voltage sensor
#   that is sensing 7.4v.
#   This tester depends on connecting the receiver to the PC through
#   a FTDI-like connection converting the receiver's serial output
#   to a USB serial device/port. See README.md for connection.
#
#####################################################################

import serial
import time

IBUS_CMD_DISCOVER = 8
IBUS_CMD_SENSOR_TYPE = 9
IBUS_CMD_SENSOR_READ = 10

IBUS_SENSOR_ID = 1          # Sensor #1
IBUS_SENSOR_TYPE = 3        # External voltage sensor
#IBUS_SENSOR_TYPE = 6        # Battery remaining percentage
IBUS_SENSOR_BYTES = 2       # 2-byte sensor data



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

def send_packet(serial_out, command, sensor_id, *data_bytes):
    '''
    Build and send a data packet to the RC receiver.
    The function calculated packet length and checksum before sending to RC receiver.
    '''
    packet_length = 4 + len(data_bytes)
    cmd_sensor = (command << 4) + sensor_id
    checksum = packet_length + cmd_sensor
    for byte in data_bytes:
        checksum = checksum + byte
        
    checksum = 65535 - checksum
    
    packet = bytearray([packet_length, cmd_sensor])
    for byte in data_bytes:
        packet.append(byte)
        
    packet.append(checksum & 255)
    packet.append(checksum >> 8)
    
    #print(packet)
    serial_out.write(packet)
    # Because the receive and transmit lines are linked
    # we need to cleanup bytes just sent from the receiver buffer
    serial_out.read(packet_length)


#ser = serial.Serial('/dev/myTTY', baudrate=115200, write_timeout=0.5, timeout=0.5)
ser = serial.Serial('/dev/ttyUSB0', baudrate=115200, write_timeout=0.5, timeout=0.5)
print(ser.name, ser.baudrate, ser.bytesize, ser.parity, ser.stopbits)

now = time.time()
while True:
    length, data = get_packet(ser)
    command, sensor, checksum_ok = parse_packet(length, data)

    print('-')
    print(time.time()-now)
    now = time.time()

    if not checksum_ok:
        print('Checksum error')
        break

    if command == IBUS_CMD_DISCOVER:
        print('Discover', sensor, '[', checksum_ok, ']')
        if sensor == IBUS_SENSOR_ID:
            send_packet(ser, command, sensor)
            
    elif command == IBUS_CMD_SENSOR_TYPE:
        print('Get sensor type', sensor, '[', checksum_ok, ']')
        if sensor == IBUS_SENSOR_ID:
            send_packet(ser, command, sensor, IBUS_SENSOR_TYPE, IBUS_SENSOR_BYTES)
            
    elif command == IBUS_CMD_SENSOR_READ:
        print('Read sensor', sensor, '[', checksum_ok, ']')
        if sensor == IBUS_SENSOR_ID:
            send_packet(ser, command, sensor, 228, 2)   # 7.4v
            
    else:
        print('Undefined command')
        break

ser.close()



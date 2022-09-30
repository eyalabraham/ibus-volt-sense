# FlySky i.Bus protocol for sensors

< FlySky receiver picture >

The protocol described here applies only to FlySky FS-IA16B RC receivers as it applies to remote sensors that can connect to the receiver. The description is only of the message serial message exchange and does not go into the wiring details between the receiver and the sensor.

The serial bus follows the RS232 signaling specification but NOT the RS232 voltage level format. Voltage levels are TTL levels where MARK '1' is over 2.0v and SPACE '0' is below 0.8v.

The serial interface runs at 115200 BAUD, 8 data bits (MSB first), no parity, 1 start bit.

Based on [Single wire FlySky (IBus) telemetry](https://github.com/betaflight/betaflight/wiki/Single-wire-FlySky-(IBus)-telemetry) and on [FLYSKY IBUS INTERFACE](https://www.cloudacm.com/?p=3865)

## Message format

The i.Bus message format is a packet of bytes sent on the serial link. The packet consists of packet length header bytes, a command byte, one of more parameter bytes, and a 16-bit (2 bytes) checksum. The command byte is divided into two fields: bit 0 to 3 host the sensor ID, bits 4 to 7 hold the command.

The checksum is calculated by subtracting the sum of all packet bytes from 0xffff.

All data is represented in Little Endian format.

**general i.Bus packet format**

| Byte | i.Bus packet          |
|:----:|:---------------------:|
| 1    |     Packet length     |
| 2    | Command   / sensor ID |
| 3    | Optional parameter 1  |
|      |       ...             |
| N+2  | Optional parameter N  |
| N+3  | Checksum low byte     |
| N+4  | Checksum high byte    |

**i.Bus command list**

| Command | Meaning                                                                                                    |
|:-------:|:-----------------------------------------------------------------------------------------------------------|
| 8       | Discover sensor. Sent by the RC receiver to discover sensors on the link/bus.                              |
| 9       | Request sensor type. Sent by the RC receiver to a specific address, requesting sensor to declare its type. |
| 10      | Request measurement. Sent by RC receiver to a specific address, requesting sensor to send data.            |

## Bus discovery

Communication begins with a bus discovery. The RC receiver probes the i.Bus link for sensors with a sensor discovery command, stepping through available addresses.

> Does this discovery cycle only run once cycling through the 1 to 15 address numbers? Sensors that reply are then queried for type and then read. For any sensors that stops responding to any command the RC receiver restarts discovery cycle from the beginning. All command packets are transmitted from the Rc receiver at a rate of 133Hz. If sensor reading is taking place then the next sensor in the list is queried every 0.5 seconds. For example: sensor 1 responded and is being read, then sensor 2 is queried with a discover command every 0.5 seconds.

> The source material seems to imply that the RC receiver assigns sensors addresses. Whereas the protocol implies that sensor addresses are hard coded into the sensor, and the user needs to make sure that sensors do not have overlapping IDs. Sensor IDs and sensor type numbers do not have to match, although it makes sense to match them at least for the upper set of codes between 1 and 15. Sensor IDs (addresses) should be contiguous starting with 1, otherwise they will not be discovered.

**Discover command packet**

| Byte | i.Bus packet       |
|:----:|:------------------:|
| 1    |         4          |
| 2    |      8  /    ID    |
| 3    | Checksum low byte  |
| 4    | Checksum high byte |

**Response packet**

If a sensor exists at the ID address, it responds by echoing back the command packet.
 
## Sensor discovery

After i.Bus discovery the RC receiver sends a packet to discover the sensor type. The i.Bus receiver continues the discovery process by querying the next address. Discovery stops at the first address which does not respond.

**Sensor type command packet**

The IDs used are ID that responded in the bus discovery phase.

| Byte | i.Bus packet       |
|:----:|:------------------:|
| 1    |         4          |
| 2    |      9  /    ID    |
| 3    | Checksum low byte  |
| 4    | Checksum high byte |

**Sensor type response packet**

The sensor at address ID responds with its type

> What does byte #4 represent, source indicates it is unknown and always has value of 2? Seems to represent the length, in number of bytes, of the response value.

| Byte | i.Bus packet       |
|:----:|:------------------:|
| 1    |         6          |
| 2    |      9  /    ID    |
| 3    | Measurement type   |
| 4    |         2          |
| 5    | Checksum low byte  |
| 6    | Checksum high byte |

**i.Bus sensor type list**

Sensor type byte values are represented in header file ```sensor_type.h```

## Sensor read

The i.Bus receiver begins a continuous loop requesting measurements from each sensor.

> What happens if a sensor stops responding to sensor read requests? The RC receiver restarts discovery phase.

**Sensor read command packet**

The IDs used are ID that responded in the bus discovery phase.

| Byte | i.Bus packet       |
|:----:|:------------------:|
| 1    |         4          |
| 2    |     10  /    ID    |
| 3    | Checksum low byte  |
| 4    | Checksum high byte |

**Sensor read response packet**

The sensor at address ID responds with its data

| Byte | i.Bus packet       |
|:----:|:------------------:|
| 1    |        N+2         |
| 2    |     10  /    ID    |
| 3    | Data low byte      |
| 4    | Data high byte     |
| N    |       ...          |
| N+1  | Checksum low byte  |
| N+2  | Checksum high byte |


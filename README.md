# FlySky i.BUS voltage sensor

FlySky-compatible i.BUS voltage sensor implemented in an AVR ATmega328P. For use only with 3S and 4S LiPo batteries.

This release is a complete rewrite of the driver. It is tailored to the FlySky RC receiver sensor bus protocol. As the RC receiver sends a command packet of 4-bytes at about 130 Hz rate, the driver uses AVR Timer1 to syncronize the received bytes into 4-byte packets by identifying the ~7mSec packet gap. At 115200 baud each byte is ~87uSec and the timer is set to trip after 1mSec. If the timer trips then a gap has been detected and the receiver byte index resets to 0, which will now capture the start of the next packet after the gap. Once 4 bytes have been received it is returned for processing by the sensor code. The voltage sensor 'registers' itself with the reciver and then responds to READ VOLTAGE commands.

```
+---------+                +------------+
| AVR     |                |            |
| ATmega  |                | FS-iA6B    |
| 328P    |                | receiver   |
|         |  Diode         |            |
| Ser Tx  |---|<---+       |            |
|         |        |       | i.bus      |
| Ser Rx  |--------+-------| sensor     |
+---------+                +------------+
```

Sensor prototype installed in a SIG FourStar 20EP RC plane: 

![ibus voltage sensor prototype](./doc/ibus-voltage-sensor.png)

## Files

- ibusvsense.c        -- Main sesnor module source code
- sensor_type.h       -- i.bus sensor types
- ibus_drv.*          -- Header and source for i.bus serial driver
- util.*              -- Utility functions
- test/               -- Some test code in Python
- doc/                -- Schematic and image

## Resources

[Single wire FlySky I.Bus telemetry](https://github.com/betaflight/betaflight/wiki/Single-wire-FlySky-(IBus)-telemetry)




# FlySky i.BUS sensor tester

## FlySky receiver emulation

Host software in Python to test the i.Bus sensor by emulating the FlySky receiver.

Use USB to RS-232 (FTDY type cable) instead of a FlySky receiver to connect host to the sensor. The programs here were used to test the sensor during development and program calibration values.

The sensor connects to the FlySky receiver through a single wire using half duplex communication to interleave receive and transmitted data on the same line. When connecting the USB-Serial device both receive and transmit lines on the sensor are connected.

```

  +--------+         +--------+
  |        |         |        |
  | USB    +---Rx----+        |
  | to     +---Tx----+ Sensor |
  | Serial |         |        |
  |        |         |        |
  +--------+         +--------+

```

I use ```interceptty``` to monitor data exchange between host and sensor:

```
sudo interceptty -s 'ispeed 115200 ospeed 115200' /dev/ttyUSB0  /dev/myTTY
```

## Sensor emulator

Python code that emulates a sensors, or sensors, for connecting to a FlySky receiver using USB to RS-232 FTDY type cable.

The setup allows examining and exercising the receiver to characterize its behavior.

```

  +--------+               +----------+
  | PC     |               |          |
  | USB    +---Rx-------+  |          |
  | to     |            |  |          |
  | Serial +---Tx---|<--+--+ FlySky   |
  |        |      diode    | receiver |
  |        |               |          |
  +--------+               +----------+

```



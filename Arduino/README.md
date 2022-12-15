# Simple IoT Arduino examples

This directory contains reference implementations of Arduino firmware that can
be connected to the
[MCU Serial Client](https://docs.simpleiot.org/docs/user/mcu.html) in SIOT
system.

- install [arduino-cli](https://arduino.github.io/arduino-cli)
- `siot_setup` (only run once)
- `siot_protobuf_gen` (can be used to update generated protobuf files)
- edit `envsetup.sh` to make sure you are set up for the correct board
- `siot_build_siot_mcu_serial` (build siot MCU example)

When connected to the Simple IoT Go application on a host system, disable the
client before trying to program the MCU.

## Projects

- `siot-mcu-cobs`: simple ascii messages wrapped with cobs
- `siot-mcu-serial`: simple example of sending points over USB
- `siot-mcu-serial-hr`: example of sending both normal and high rate points over
  USB
- `siot-mcu-serial-hr-fixed`: encode once, and send point as fast as possible

## Performance results

All testing was done with a SEEED Studio XIAO SAMD21 board (48Mhz). Assume
packet is 30 bytes. SAMD21 USB connection is full speed (12Mbps)

- `siot-mcu-serial-hr`
  - could send about 1451 pts/second (348,241 Kbps)
  - disabling sending, could encode 1618 pts/second
- `siot-mcu-serial-hr-fixed`
  - could send about 11,000 pts/second (2.64Mbps)

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

## Projects

- `siot-mcu-cobs`: simple ascii messages wrapped with cobs
- `siot-mcu-serial`: simple example of sending points over USB
- `siot-mcu-serial-hr`: example of sending both normal and high rate points over
  USB

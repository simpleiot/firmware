siot_setup() {
  git submodule update --init
  arduino-cli lib install PacketSerial
}

siot_protobuf_gen() {
  (cd libraries/pb && python ../../nanopb/generator/nanopb_generator.py ./*.proto)
}

siot_ctags() {
  arduino-ctags -R
}

# the following function needs to be run any time you update nanopb
# to copy the library files to where arduino-cli can find them.
siot_install_nanopb_library() {
  cp nanopb/*.h libraries/nanopb/
  cp nanopb/*.c libraries/nanopb/
}

siot_build_siot_mcu_serial() {
  arduino-cli compile \
    Arduino/SIOT_MCU_serial \
    --libraries libraries \
    -b arduino:avr:uno
}

siot_upload_siot_mcu_serial() {
  siot_build_siot_mcu_serial &&
    arduino-cli upload \
      Arduino/SIOT_MCU_serial \
      -b arduino:avr:uno \
      -p /dev/ttyACM0
}

siot_build_siot_mcu_cobs() {
  arduino-cli compile \
    Arduino/siot-mcu-cobs \
    -b arduino:avr:uno
}

siot_upload_siot_mcu_cobs() {
  siot_build_siot_mcu_cobs &&
    arduino-cli upload \
      Arduino/siot-mcu-cobs \
      -b arduino:avr:uno \
      -p /dev/ttyACM0
}

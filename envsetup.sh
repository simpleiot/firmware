# mkr1000 appears to work with a SEEED XIAO SAMD21 board

siot_mcu_board=arduino:samd:mkr1000
siot_mcu_port=/dev/ttyACM1
#siot_mcu_board=arduino:avr:uno

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
    Arduino/siot-mcu-serial \
    --libraries libraries \
    -b $siot_mcu_board
}

siot_upload_siot_mcu_serial() {
  siot_build_siot_mcu_serial &&
    arduino-cli upload \
      Arduino/siot-mcu-serial \
      -b $siot_mcu_board \
      -p $siot_mcu_port
}

siot_build_siot_mcu_cobs() {
  arduino-cli compile \
    Arduino/siot-mcu-cobs \
    -b $siot_mcu_board
}

siot_upload_siot_mcu_cobs() {
  siot_build_siot_mcu_cobs &&
    arduino-cli upload \
      Arduino/siot-mcu-cobs \
      -b $siot_mcu_board \
      -p $siot_mcu_port
}

siot_pb_test_run() {
  (cd cmd/pb-test && gcc -I ../../libraries/pb \
    -I ../../libraries/nanopb \
    -o pb-test \
    ../../libraries/nanopb/pb_common.c \
    ../../libraries/nanopb/pb_encode.c \
    ../../libraries/pb/point.pb.c \
    ../../libraries/pb/timestamp.pb.c \
    ../../libraries/pb/siot_serial.pb.c \
    main.c &&
    ./pb-test)
}

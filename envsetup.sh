siot_protobuf_gen() {
  (cd library/pb && python ../../nanopb/generator/nanopb_generator.py ./*.proto)
}

siot_build_siot_mcu_serial() {
  (cd Arduino/SIOT_MCU_serial && arduino-cli compile \
    --libraries ../../libraries \
    -b arduino:avr:uno)
}

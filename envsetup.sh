siot_setup() {
  git submodule update --init
}

siot_protobuf_gen() {
  (cd library/pb && python ../../nanopb/generator/nanopb_generator.py ./*.proto)
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
  (cd Arduino/SIOT_MCU_serial && arduino-cli compile \
    --libraries ../../libraries \
    -b arduino:avr:uno)
}

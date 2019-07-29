

all: firmware.bin

firmware.bin: *.ino
	particle compile boron ./ --saveTo firmware.bin

flash: firmware.bin
	particle flash --usb firmware.bin

console:
	particle serial monitor --follow



all: firmware.bin

firmware.bin: *.ino
	particle compile boron ./ --saveTo firmware.bin

flash: firmware.bin
	particle usb dfu && sleep 2
	particle flash --usb firmware.bin

console:
	particle serial monitor --follow


clean:
	rm firmware.bin



all: firmware.bin

firmware.bin: *.ino
	particle compile argon ./ --saveTo firmware.bin

flash: firmware.bin
	particle flash --usb firmware.bin

console:
	particle serial monitor --follow


clean:
	rm firmware.bin

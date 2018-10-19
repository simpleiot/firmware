


all:
	particle compile electron ./ --saveTo firmware.bin

flash:
	particle flash --usb firmware.bin

console:
	particle serial monitor --follow

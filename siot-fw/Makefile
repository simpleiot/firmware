# this makefile is set up for cloud compiling
# use particle workbench to compile locally
# this currently does not work -- kept for reference only

all: firmware.bin

firmware.bin: src/*
	ctags -R
	particle compile argon ./src ./lib --saveTo firmware.bin

flash: firmware.bin
	particle usb dfu && sleep 2
	particle flash --usb firmware.bin

console:
	particle serial monitor --follow


clean:
	rm firmware.bin

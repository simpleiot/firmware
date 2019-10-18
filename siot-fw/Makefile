# this makefile is set up for cloud compiling
# use particle workbench to compile locally

all: firmware.bin

firmware.bin: src/*
	ctags -R
	particle compile argon --saveTo firmware.bin

flash: firmware.bin
	particle usb dfu && sleep 2
	particle flash --usb firmware.bin

console:
	particle serial monitor --follow

format:
	find -iname *.h -o -iname *.cpp | xargs clang-format -i

clean:
	rm firmware.bin

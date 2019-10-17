
#include "OneWireDevice.h"
#include "print.h"

OneWireDevice::OneWireDevice(int busIndex_, uint64_t id_):
	busIndex(busIndex_),
	id(id_),
	initialized(false)
{
}

bool OneWireDevice::same(OneWireDevice d)
{
	return id == d.id && busIndex == d.busIndex;
}

uint8_t OneWireDevice::family()
{
	return uint8_t(id);
}

String OneWireDevice::string()
{
	return String::format("B%i: %s", busIndex, formatU64Hex(id).c_str());
}

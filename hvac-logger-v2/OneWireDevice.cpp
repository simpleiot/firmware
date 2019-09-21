
#include "OneWireDevice.h"
#include "print.h"

OneWireDevice::OneWireDevice(int busIndex_, uint64_t id_):
	busIndex(busIndex_),
	id(id_)
{
}

bool OneWireDevice::same(OneWireDevice d)
{
	return id == d.id && busIndex == d.busIndex;
}

String OneWireDevice::string()
{
	return String::format("B%i: %s", busIndex, formatU64Hex(id).c_str());
}

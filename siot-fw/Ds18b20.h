#ifndef DS18B20_H
#define DS18B20_H

#include "OneWireBus.h"
#include "Sample.h"

class Ds18b20
{
	OneWireBus *_bus;
	uint64_t _id;

	// must pass in 8 byte array
	int _readScratchpad(uint8_t *data);
	int _setResolution();

	public:
	Ds18b20(OneWireBus *bus, uint64_t id);
	int init();
	int read(Sample *sample);
};

#endif



#ifndef DS2438_H
#define DS2438_H

#include "OneWireBus.h"
#include "Sample.h"

class Ds2438
{
	OneWireBus *_bus;
	uint64_t _id;


	int _readScratchpad(uint8_t *spad);

	public:
	Ds2438(OneWireBus *bus, uint64_t id);
	int init();
	int read(Sample *sample);
};

#endif

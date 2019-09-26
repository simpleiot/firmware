#include <inttypes.h>

#include "Ds18b20.h"
#include "crc.h"

#define CMD_CONVERT 0x44
#define CMD_WRITE_SCRATCHPAD 0x4E
#define CMD_READ_SCRATCHPAD 0xBE
#define CMD_COPY_SCRATCHPAD 0x48

Ds18b20::Ds18b20(OneWireBus *bus, uint64_t id):
	_bus(bus),
	_id(id)
{
}

int Ds18b20::_readScratchpad(uint8_t *spad)
{
	uint8_t spadCrc[9];

	uint8_t cmd[] = {CMD_READ_SCRATCHPAD};

	int ret = _bus->txMatch(_id, cmd, sizeof(cmd), spadCrc, sizeof(spadCrc));

	if (ret) {
		return ret;
	}

	Serial.println("");

	if (!CheckCRC(spadCrc, sizeof(spadCrc))) {
		return OneWireErrorCrc;
	}

	memcpy(spad, spadCrc, 8);
	return 0;
}

int Ds18b20::_setResolution()
{
	uint8_t spad[8];

	int ret = _readScratchpad(spad);

	if (ret) {
		return ret;
	}

	Serial.print("scratchpad data: ");
	for (unsigned int i=0; i<sizeof(spad); i++) {
		Serial.printf("%02x ", spad[i]);
	}
	Serial.println("");
}


int Ds18b20::init()
{
	_bus->select(true);
	int ret = _setResolution();
	_bus->select(false);
	return ret;
}

int Ds18b20::read(Sample *sample)
{
	_bus->select(true);
	Serial.println("reading temp sensor");
	_bus->select(false);
	return 0;
}

#include <inttypes.h>

#include "Ds18b20.h"
#include "crc.h"
#include "print.h"

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

	if (spad[4] != 0x3f) {
		Serial.println("setting resolution to 10 bits");
		uint8_t cmd[4] = {CMD_WRITE_SCRATCHPAD, 0xff, 0xff, 0x3f};
		ret = _bus->txMatch(_id, cmd, sizeof(cmd), NULL, 0);
	}

	return ret;
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
	int err = 0;
	float temp;
	_bus->select(true);
	uint8_t cmd[] = {CMD_CONVERT};
	err = _bus->txMatch(_id, cmd, sizeof(cmd), NULL, 0);
	if (err) {
		goto read_done;
	}
	delay(200);

	uint8_t spad[8];
	err = _readScratchpad(spad);

	if (err) {
		goto read_done;
	}

	temp = int(spad[1]) << 8 | int(spad[0]);
	temp = temp/16;

	sample->id = formatU64Hex(_id);
	sample->type = String("temp");
	sample->value = temp;

read_done:
	_bus->select(false);
	return 0;
}

#include <inttypes.h>

#include "Ds2438.h"
#include "crc.h"
#include "print.h"

#define CMD_CONVERT_VOLTAGE 0xB4
#define CMD_RECALL_MEMORY_PAGE 0xB8
#define CMD_READ_SCRATCHPAD 0xBE
#define CMD_WRITE_SCRATCHPAD 0x4E
#define CMD_COPY_SCRATCHPAD 0x48

Ds2438::Ds2438(OneWireBus *bus, uint64_t id):
	_bus(bus),
	_id(id)
{
}

int Ds2438::_readScratchpad(uint8_t *spad)
{
	uint8_t spadCrc[9];

	uint8_t cmd[] = {CMD_READ_SCRATCHPAD, 0};

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


int Ds2438::read(Sample *sample)
{
	int err = 0;
	uint16_t voltReg;
	float volts;
	_bus->select(true);

	// enable general purpose A/D and disable the rest of the stuff
	// on the chip
	uint8_t cmd3[] = {CMD_CONVERT_VOLTAGE};
	uint8_t cmd4[] = {CMD_RECALL_MEMORY_PAGE, 0};

	err = _bus->txMatch(_id, cmd3, sizeof(cmd3), NULL, 0);
	if (err) {
		goto read_done;
	}

	delay(10);

	err = _bus->txMatch(_id, cmd4, sizeof(cmd4), NULL, 0);
	if (err) {
		goto read_done;
	}

	uint8_t spad[8];
	err = _readScratchpad(spad);

	if (err) {
		goto read_done;
	}

	voltReg = uint16_t(spad[4]) << 8 | uint16_t(spad[3]);

	// A/D sometimes reads 0x3ff when no voltage is applied
	if (voltReg == 0x3ff) {
		voltReg = 0;
	}

	volts = voltReg * 0.01;

	sample->id = formatU64Hex(_id);
	sample->type = String("volt");
	sample->value = volts;

read_done:
	_bus->select(false);
	return err;
}

int Ds2438::init()
{
	int err = 0;
	_bus->select(true);
	// enable general purpose A/D and disable the rest of the stuff
	// on the chip
	uint8_t cmd[] = {CMD_WRITE_SCRATCHPAD, 0, 0x0};

	uint8_t cmd2[] = {CMD_COPY_SCRATCHPAD, 0};
	err = _bus->txMatch(_id, cmd, sizeof(cmd), NULL, 0);
	if (err) {
		goto init_done;
	}

	err = _bus->txMatch(_id, cmd2, sizeof(cmd2), NULL, 0);
	if (err) {
		goto init_done;
	}

init_done:
	_bus->select(false);
	return err;
}

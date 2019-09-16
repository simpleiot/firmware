
#include <inttypes.h>

#define RESET_RET_DEVICE_DETECTED 0
#define RESET_RET_NO_DEVICE -1
#define RESET_RET_SHORT_DETECTED -2

class OneWireBus
{
	int _selectPin;
	int _i2cAddress;

	int _reset();
	uint8_t _waitIdle();

	public:
	OneWireBus(int selectPin, int i2cAddress);

	void search();
};

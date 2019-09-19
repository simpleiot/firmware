
#include <inttypes.h>

// errors that may be returned
enum OneWireError {
	OneWireErrorNoDevice = -1,
	OneWireErrorShortDetected = -2,
	OneWireErrorTimeout = -3,
	OneWireErrorDevicesDisappeared = -4,
	OneWireErrorCrc = -5,
	OneWireErrorI2C = -6,
	OneWireErrorLastDevice = -7
};

enum OneWireFamCode {
	OneWireFamTemp = 0x28,
	OneWireFamAD = 0x26
};

typedef struct {
	bool GotZero;
	bool GotOne;
	bool Taken;
	int err;
} TripletReturn;

typedef struct {
	uint8_t status;
	int err;
} WaitReturn;

typedef struct {
	uint64_t device;
	int err;
} SearchReturn;

char * OneWireErrorString(int err);

class OneWireBus
{
	int _selectPin;
	int _i2cAddress;
	char * _name;
	int _searchLastDiscrepency;
	uint64_t _searchLastDevice;

	int _reset();
	WaitReturn _waitIdle();
	TripletReturn _searchTriplet(uint8_t direction);

	public:
	OneWireBus(char *name, int selectPin, int i2cAddress);
	// search is designed to be call repeatably and returns devices IDs
	// found. After it hits the last devices, the search is reset internally.
	SearchReturn search();

	int tx(uint8_t *w, int wCnt, uint8_t *r, int rSize);
};

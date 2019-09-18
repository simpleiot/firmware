
#include <inttypes.h>

// errors that may be returned
enum OneWireError {
	OneWireErrorNoDevice = -1,
	OneWireErrorShortDetected = -2,
	OneWireErrorTimeout = -3,
	OneWireErrorDevicesDisappeared = -4,
	OneWireErrorCrc = -5,
	OneWireErrorI2C = -6
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


char * OneWireErrorString(int err);

class OneWireBus
{
	int _selectPin;
	int _i2cAddress;

	int _reset();
	WaitReturn _waitIdle();
	TripletReturn _searchTriplet(uint8_t direction);

	public:
	OneWireBus(int selectPin, int i2cAddress);

	int search();
	int tx(uint8_t *w, int wCnt, uint8_t *r, int rSize);
};

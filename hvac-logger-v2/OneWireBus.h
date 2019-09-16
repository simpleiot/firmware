
#include <inttypes.h>

// errors that may be returned
#define RESET_RET_NO_DEVICE -1
#define RESET_RET_SHORT_DETECTED -2

typedef struct {
	bool TripletGotZero;
	bool TripletGotOne;
	bool TripletTaken;
} TripletReturn;

class OneWireBus
{
	int _selectPin;
	int _i2cAddress;

	int _reset();
	uint8_t _waitIdle();
	TripletReturn _searchTriplet(uint8_t direction);

	public:
	OneWireBus(int selectPin, int i2cAddress);

	int search();
	int tx(uint8_t *w, int wCnt, uint8_t *r, int rSize);
};

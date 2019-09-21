#include <Particle.h>
#include <inttypes.h>
#include <vector>

#include "OneWireBus.h"
#include "OneWireDevice.h"

class OneWireErrorCounts
{
	public:
	int shortDetected;
	int timeout;
	int deviceDisappeared;
	int crc;
	int i2c;

	OneWireErrorCounts();
	void error(int error);
	String string();
};

class OneWireManager
{
	std::vector<OneWireDevice> _devices;
	std::vector<OneWireBus*> _busses;
	OneWireErrorCounts _errorCounts;

	int _findDevice(OneWireDevice d);

	public:
	OneWireManager();
	void addBus(OneWireBus *bus);
	bool search();
	OneWireErrorCounts getErrors();
};

#include <Particle.h>
#include <inttypes.h>
#include <vector>

#include "OneWireBus.h"
#include "OneWireDevice.h"

class OneWireManager
{
	std::vector<OneWireDevice> _devices;
	std::vector<OneWireBus*> _busses;

	int _findDevice(OneWireDevice d);

	public:
	OneWireManager();
	void addBus(OneWireBus *bus);
	bool search();
};

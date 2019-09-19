#include <Particle.h>
#include <inttypes.h>
#include <vector>

#include "OneWireBus.h"

class OneWireManager
{
	std::vector<uint64_t> _devices;
	std::vector<OneWireBus*> _busses;

	public:
	OneWireManager();
	void addBus(OneWireBus *bus);
	int search();
};

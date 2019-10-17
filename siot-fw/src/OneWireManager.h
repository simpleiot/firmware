#ifndef ONEWIREMANAGER_H
#define ONEWIREMANAGER_H

#include <Particle.h>
#include <inttypes.h>
#include <vector>

#include "OneWireDevice.h"
#include "OneWireBus.h"
#include "Sample.h"

class OneWireManager
{
	// internal state
	unsigned int _readIndex;
	std::vector<OneWireDevice> _devices;
	std::vector<OneWireBus*> _busses;
	OneWireErrorCounts _errorCounts;

	int _findDevice(OneWireDevice d);
	int _initDevice(OneWireDevice *d);

	public:
	OneWireManager();
	void addBus(OneWireBus *bus);
	int init();
	bool search();

	/* read is used to read data from devices on the one wire bus. This
	 * function should be called continually until it returns OneWireLastData,
	 * at that point you'll know you reached the end of the list of devices.
	 */
	int read(Sample *sample);
	OneWireErrorCounts getErrors();
};

#endif

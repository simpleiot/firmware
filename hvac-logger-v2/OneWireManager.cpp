#include "OneWireManager.h"
#include "print.h"

OneWireManager::OneWireManager()
{
}

void OneWireManager::addBus(OneWireBus *bus)
{
	_busses.push_back(bus);
}

// returns index if device found, otherwise -1
int OneWireManager::_findDevice(OneWireDevice d)
{
	for (unsigned int i=0; i < _devices.size(); i++) {
		if (_devices[i].same(d)) {
			return i;
		}
	}

	return -1;

}

bool OneWireManager::search()
{
	bool modified = false;
	std::vector<bool> found(_devices.size(), false);

	for (unsigned int i=0; i < _busses.size(); i++) {
		while (true) {
			SearchReturn ret = _busses[i]->search();
			if (ret.err && ret.err != OneWireErrorLastDevice &&
					ret.err != OneWireErrorNoDevice) {
				char *errS = OneWireErrorString(ret.err);
				Serial.printf("Search returned an error: %s\n", errS);
				break;
			}

			if (ret.err == OneWireErrorNoDevice) {
				break;
			}


			OneWireDevice d = OneWireDevice(i, ret.device);

			int index = _findDevice(d);

			if (index < 0) {
				// did not find device so adding it
				_devices.push_back(d);
				Serial.printf("Found a new device: %s\n",
						d.string().c_str());
				modified = true;
			} else {
				found[index] = true;
			}

			if (ret.err == OneWireErrorLastDevice) {
				break;
			}
		};
	}

	// check for devices that have disappeared
	for (int i=found.size()-1; i >= 0; i--) {
		if (!found[i]) {
			Serial.printf("Device dissapeared: %s\n",
					_devices[i].string().c_str());

			_devices.erase(_devices.begin() + i);
			modified = true;
		}
	}

	return modified;
}

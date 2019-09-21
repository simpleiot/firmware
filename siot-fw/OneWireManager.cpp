#include "OneWireManager.h"
#include "print.h"

OneWireErrorCounts::OneWireErrorCounts() :
	shortDetected(0),
	timeout(0),
	deviceDisappeared(0),
	crc(0),
	i2c(0)
{
}

void OneWireErrorCounts::error(int error)
{
	switch (error) {
		case OneWireErrorShortDetected:
			shortDetected++;
			break;
		case OneWireErrorTimeout:
			timeout++;
			break;
		case OneWireErrorDevicesDisappeared:
			deviceDisappeared++;
			break;
		case OneWireErrorCrc:
			crc++;
			break;
		case OneWireErrorI2C:
			i2c++;
			break;
		default:
			Serial.printf("Warning: OneWireErrorCounts, don't know how to handle: %i\n", error);

	}
}

String OneWireErrorCounts::string()
{
	return String::format("shorts: %i\ntimeouts: %i\ndisappeared: %i\ncrc: %i\ni2c: %i\n",
			shortDetected,
			timeout,
			deviceDisappeared,
			crc,
			i2c);
}

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
		int errCnt = 0;
		while (true) {
			SearchReturn ret = _busses[i]->search();
			if (ret.err && ret.err != OneWireErrorLastDevice &&
					ret.err != OneWireErrorNoDevice) {
				char *errS = OneWireErrorString(ret.err);
				Serial.printf("Search returned an error: %s\n", errS);

				// keep track of error counts
				_errorCounts.error(ret.err);

				errCnt++;
				if (errCnt > 5) {
					Serial.println("too many errors while searching bus, breaking");
					break;
				}
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

OneWireErrorCounts OneWireManager::getErrors()
{
	return _errorCounts;
}

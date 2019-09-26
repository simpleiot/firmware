#include "OneWireManager.h"
#include "Ds18b20.h"
#include "print.h"

OneWireManager::OneWireManager():
	_readIndex(0)
{
}

void OneWireManager::addBus(OneWireBus *bus)
{
	_busses.push_back(bus);
}


int OneWireManager::init()
{
	int err = 0;
	for (unsigned int i=0; i < _busses.size(); i++) {
		int e = _busses[i]->init();
		if (e) {
			err = e;
		}
	}

	return err;
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

int OneWireManager::_initDevice(OneWireDevice *d)
{
	Serial.printf("Initializing %s\n", d->string().c_str());
	switch (d->family()) {
		case OneWireFamTemp:
			{
				Ds18b20 s = Ds18b20(_busses[d->busIndex], d->id);
				return s.init();
			}
		default:
			return OneWireErrorUnsupported;
	}
}

bool OneWireManager::search()
{
	bool modified = false;
	std::vector<bool> found(_devices.size(), false);

	for (unsigned int i=0; i < _busses.size(); i++) {
		int errCnt = 0;
		while (true) {
			SearchReturn ret = _busses[i]->search();
			if (ret.err && ret.err != OneWireNoMoreDevices &&
					ret.err != OneWireErrorNoDevice) {
				const char *errS = OneWireErrorString(ret.err);
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

			if (ret.err == OneWireNoMoreDevices) {
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

int OneWireManager::read(Sample *sample)
{
	int ret = 0;

	if (_readIndex >= _devices.size()) {
		_readIndex = 0;
		return OneWireNoMoreData;
	}

	OneWireDevice *d = &_devices[_readIndex];

	if (!d->initialized) {
		ret = _initDevice(d);
		if (ret == OneWireErrorUnsupported) {
			Serial.printf("init not supported for: %s\n", d->string().c_str());
			d->initialized = true;
		} else if (ret) {
			Serial.printf("init error %s for: %s\n",
					OneWireErrorString(ret),
					d->string().c_str());
		} else {
			d->initialized = true;
		}
	}

	if (d->initialized) {
		uint8_t family = d->family();

		switch (family) {
			case OneWireFamTemp:
				{
					Ds18b20 s = Ds18b20(_busses[d->busIndex], d->id);
					ret = s.read(sample);
					break;
				}
			case OneWireFamAD:
				ret = OneWireErrorUnsupported;
				break;
			default:
				Serial.printf("Unknown one wire fam code: 0x%X\n", family);
				ret = OneWireErrorUnsupported;
		}
	}

	_readIndex++;
	_errorCounts.error(ret);

	return ret;
}

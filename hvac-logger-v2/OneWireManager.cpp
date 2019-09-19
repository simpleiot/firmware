#include "OneWireManager.h"
#include "print.h"

OneWireManager::OneWireManager()
{
}

void OneWireManager::addBus(OneWireBus *bus)
{
	_busses.push_back(bus);
}

int OneWireManager::search()
{
	for (int i=0; i < _busses.size(); i++) {
		Serial.printf("Scanning one wire bus %s: \n", _busses[i]->getName());
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

			Serial.println(formatU64Hex(ret.device));
			if (ret.err == OneWireErrorLastDevice) {
				break;
			}
		};
	}
}

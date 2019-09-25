#include <OneWireErrors.h>

const char * OneWireErrorString(int err)
{
	switch (err) {
		case OneWireErrorNoDevice:
			return "no device";
			break;
		case OneWireErrorShortDetected:
			return "short detected";
			break;
		case OneWireErrorTimeout:
			return "timeout";
			break;
		case OneWireErrorDevicesDisappeared:
			return "device disappeared";
			break;
		case OneWireErrorCrc:
			return "crc";
			break;
		case OneWireErrorI2C:
			return "i2c";
			break;
		case OneWireErrorUnsupported:
			return "unsupported";
			break;

		case OneWireNoMoreDevices:
			return "no more devices";
			break;
		case OneWireNoMoreData:
			return "no more data";
			break;
		default:
			return "unknown";
	}
}

OneWireErrorCounts::OneWireErrorCounts() :
	shortDetected(0),
	timeout(0),
	deviceDisappeared(0),
	crc(0),
	i2c(0),
	unsupported(0)
{
}

void OneWireErrorCounts::error(int error)
{
	switch (error) {
		case 0:
			break;
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
		case OneWireErrorUnsupported:
			unsupported++;
			break;
		default:
			Serial.printf("Warning: OneWireErrorCounts, don't know how to handle: %i\n", error);

	}
}

String OneWireErrorCounts::string()
{
	return String::format("shorts: %i\ntimeouts: %i\ndisappeared: %i\ncrc: %i\ni2c: %i\nunsupported: %i\n",
			shortDetected,
			timeout,
			deviceDisappeared,
			crc,
			i2c,
			unsupported);
}

#include "OneWireBus.h"
#include "Particle.h"

OneWireBus::OneWireBus(int selectPin, int i2cAddress):
	_selectPin(selectPin),
	_i2cAddress(i2cAddress)
{
}

const uint8_t cmdReset       = 0xf0; // reset ds248x
const uint8_t cmdSetReadPtr  = 0xe1; // set the read pointer
const uint8_t cmdWriteConfig = 0xd2; // write the device configuration
const uint8_t cmdAdjPort     = 0xc3; // adjust 1-wire port
const uint8_t cmd1WReset     = 0xb4; // reset the 1-wire bus
const uint8_t cmd1WBit       = 0x87; // perform a single-bit transaction on the 1-wire bus
const uint8_t cmd1WWrite     = 0xa5; // perform a byte write on the 1-wire bus
const uint8_t cmd1WRead      = 0x96; // perform a byte read on the 1-wire bus
const uint8_t cmd1WTriplet   = 0x78; // perform a triplet operation (2 bit reads, a bit write)

const uint8_t regDCR    = 0xc3; // read ptr for device configuration register
const uint8_t regStatus = 0xf0; // read ptr for status register
const uint8_t regRDR    = 0xe1; // read ptr for read-data register
const uint8_t regPCR    = 0xb4; // read ptr for port configuration register

const uint8_t status1WB = (1<<0); // 1-wire line is busy
const uint8_t statusPPD = (1<<1); // presence-pulse detect
const uint8_t statusSD = (1<<2);  // short detected
const uint8_t statusLL = (1<<3);  // logic level
const uint8_t statusRST = (1<<4); // device reset has occured
const uint8_t statusSBR = (1<<5); // single bit result
const uint8_t statusTSB = (1<<6); // logic level of 2nd bit of 1-wire triplet command
const uint8_t statusDIR = (1<<7); // search direction that was taken by 3rd bit of 1-wire triplet

int OneWireBus::_reset()
{
	Wire.beginTransmission(_i2cAddress);
	Wire.write(cmd1WReset);
	Wire.endTransmission();
	uint8_t status = OneWireBus::_waitIdle();

	if ((status & statusSD) != 0) {
		Serial.println("short detected on bus");
		return RESET_RET_SHORT_DETECTED;
	}

	if ((status & statusPPD) == 0) {
		Serial.println("no device detected");
		return  RESET_RET_NO_DEVICE;

	}

	return 0;
}

uint8_t OneWireBus::_waitIdle()
{
	for (int i=0; i<3; i++) {
		delay(1);
		int cnt = Wire.requestFrom(_i2cAddress, 1);
		if (cnt <= 0) {
			continue;
		}
		uint8_t c = Wire.read();
		if ((c & status1WB) == 0) {
			return c;
		}
	}

	Serial.println("_waitIdle timed out");
	return 0;
}

int OneWireBus::search()
{
	int ret = 0;
	int lastDiscrepency = -1;
	uint64_t lastDevice;

	Serial.println("Searching 1-wire bus");
	digitalWrite(_selectPin, HIGH);

	// Loop to do the search -- each iteration detects one device
	while (true) {
		// issue a search command
		uint8_t cmd[] = {cmdReset};

		ret = tx(cmd, sizeof(cmd), NULL, 0);

		if (ret) {
			goto search_error;
		}

		// loop to accumulate the 64-bits of an ID
		int discrepancy = -1;
		uint64_t device;
		uint8_t idBytes[8];
		for (int bit = 0; bit < 64; bit++) {
			uint8_t dir;
			if (bit < lastDiscrepency) {
				// we haven't reached the last discrepancy yet, so we
				// need to repeat the bits of the last device
				dir = (lastDevice >> bit) & 1;
			} else if (bit == lastDiscrepency) {
				// we reached the bit where we picked 0 last time and
				// now we need 1.
				dir = 1;
			}
		}

		break;
	}


search_error:
	digitalWrite(_selectPin, LOW);
	return ret;
}

int OneWireBus::tx(uint8_t *w, int wCnt, uint8_t *r, int rSize)
{
	int ret = _reset();

	Serial.println("reset returned: ");
	Serial.println(ret);

	if (ret) {
		return ret;
	}

	// send bytes onto 1-wire bus
	Wire.beginTransmission(_i2cAddress);
	Wire.write(cmd1WWrite);
	for (int i=0; i<wCnt; i++) {
		Wire.write(w[i]);
	}
	Wire.endTransmission();

}


TripletReturn OneWireBus::_searchTriplet(uint8_t direction)
{
	uint8_t dir;
	if (direction != 0) {
		dir = 0x80;
	}

	Wire.beginTransmission(_i2cAddress);
	Wire.write(cmd1WTriplet);
	Wire.write(dir);
	int status = _waitIdle();
}

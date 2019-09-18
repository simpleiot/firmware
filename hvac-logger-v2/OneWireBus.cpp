#include "Particle.h"
#include "OneWireBus.h"
#include "crc.h"
#include "debug.h"
#include "print.h"

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

char * OneWireErrorString(int err)
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
		default:
			return "unknown";
	}
}

int OneWireBus::_reset()
{
	Wire.beginTransmission(_i2cAddress);
	Wire.write(cmd1WReset);
	Wire.endTransmission();
	WaitReturn waitRet = OneWireBus::_waitIdle();

	if (waitRet.err) {
		return waitRet.err;
	}

	if ((waitRet.status & statusSD) != 0) {
		Serial.println("short detected on bus");
		return OneWireErrorShortDetected;
	}

	if ((waitRet.status & statusPPD) == 0) {
		Serial.println("no device detected");
		return  OneWireErrorNoDevice;

	}

	return 0;
}

WaitReturn OneWireBus::_waitIdle()
{
	WaitReturn ret = {0,0};

	for (int i=0; i<10; i++) {
		delayMicroseconds(100);
		int cnt = Wire.requestFrom(_i2cAddress, 1);
		if (cnt <= 0) {
			continue;
		}
		ret.status = Wire.read();
		if ((ret.status & status1WB) == 0) {
			return ret;
		}
	}

	ret.err = OneWireErrorTimeout;

	Serial.println("_waitIdle timed out");
	return ret;
}

int OneWireBus::search()
{
	int err = 0;
	int lastDiscrepency = -1;
	uint64_t lastDevice;

	Serial.println("Searching 1-wire bus");
	digitalWrite(_selectPin, HIGH);

	// Loop to do the search -- each iteration detects one device
	while (true) {
		digitalWrite(PIN_BLACK, HIGH);
		Serial.println("search loop");
		// issue a search command
		uint8_t cmd[] = {cmdReset};

		err = tx(cmd, sizeof(cmd), NULL, 0);

		if (err) {
			goto search_error;
		}

		// loop to accumulate the 64-bits of an ID
		int discrepancy = -1;
		uint64_t device = 0;
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

			// Perform triplet operation
			digitalWrite(PIN_GREEN, HIGH);
			TripletReturn tripRet = _searchTriplet(dir);
			digitalWrite(PIN_GREEN, LOW);

			Serial.print("bit: ");
			Serial.print(bit);
			Serial.print(" ->");
			Serial.print(tripRet.GotZero);
			Serial.print(tripRet.GotOne);
			Serial.print(tripRet.Taken);
			Serial.println("");

			if (tripRet.err) {
				err = tripRet.err;
				goto search_error;
			}

			if (!tripRet.GotZero && !tripRet.GotOne) {
				err = OneWireErrorDevicesDisappeared;
				goto search_error;
			}

			if (tripRet.GotZero && tripRet.GotOne && !tripRet.Taken) {
				/*
				Serial.print("discrepancy at bit pos: ");
				Serial.println(bit);
				*/
				discrepancy = bit;
			}

			device |= uint64_t(tripRet.Taken) << bit;

			if ((bit&7) == 7) {
				idBytes[bit>>3] = device >> (bit-7);
			}
		}

		Serial.print("Found device: ");
		for (int i=sizeof(idBytes)-1; i >= 0 ; i--) {

			Serial.printf("%.2x", idBytes[i]);
		}
		Serial.println("");

		Serial.print("device: ");
		print64(device);
		Serial.println("");

		// Verify the CRC and record the device if we got it right.
		if (!CheckCRC(idBytes, sizeof(idBytes))) {
			err = OneWireErrorCrc;
			goto search_error;
		}

		lastDevice = device;
		lastDiscrepency = discrepancy;
		if (lastDiscrepency == -1) {
			Serial.println("No more devices");
			goto search_done;
		}

		//delay(100);

		digitalWrite(PIN_BLACK, LOW);
	}

search_error:
search_done:
	digitalWrite(_selectPin, LOW);
	digitalWrite(PIN_BLACK, LOW);
	digitalWrite(PIN_GREEN, LOW);
	return err;
}

int OneWireBus::tx(uint8_t *w, int wCnt, uint8_t *r, int rSize)
{
	int err = _reset();

	Serial.print("reset returned: ");
	Serial.println(err);

	if (err) {
		return err;
	}

	for (int i=0; i<wCnt; i++) {
		// send bytes onto 1-wire bus
		Wire.beginTransmission(_i2cAddress);
		Wire.write(cmd1WWrite);
		Wire.write(w[i]);
		err = Wire.endTransmission();

		if (err != 0) {
			Serial.print("I2C error: ");
			Serial.println(err);
			err = OneWireErrorI2C;
		}

		WaitReturn waitRet = _waitIdle();
		if (waitRet.err) {
			return waitRet.err;
		}
	}

	return err;
}


TripletReturn OneWireBus::_searchTriplet(uint8_t direction)
{
	uint8_t dir;
	if (direction != 0) {
		dir = 0x80;
	}

	TripletReturn ret;

	Wire.beginTransmission(_i2cAddress);
	Wire.write(cmd1WTriplet);
	Wire.write(dir);
	int err = Wire.endTransmission();

	if (err != 0) {
		Serial.print("I2C error: ");
		Serial.println(err);
		ret.err = OneWireErrorI2C;
		return ret;
	}

	delayMicroseconds(500);

	WaitReturn waitRet = _waitIdle();

	ret.GotZero = (waitRet.status & statusSBR) == 0;
	ret.GotOne = (waitRet.status & statusTSB) == 0;
	ret.Taken = (waitRet.status & statusDIR) != 0;
	ret.err = waitRet.err;

	return ret;
}

#include "Particle.h"
#include "OneWireBus.h"
#include "crc.h"
#include "debug.h"
#include "print.h"

OneWireBus::OneWireBus(const char *name, int selectPin, int i2cAddress):
	_searchLastDiscrepency(-1),
	_searchLastDevice(0),
	_name(name),
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
const uint8_t cmd1WMatchRom  = 0x55; // used to select a device
const uint8_t cmd1WSearch    = 0xF0; // plain search
const uint8_t cmd1WAlarmSearch = 0xEC; // only search for devices in alarm state


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
	uint8_t i2cData[] = {cmd1WReset};
	int err = i2cTx(i2cData, sizeof(i2cData), NULL, 0);

	if (err) {
		return err;
	}

	WaitReturn waitRet = OneWireBus::_waitIdle();

	if (waitRet.err) {
		return waitRet.err;
	}

	if ((waitRet.status & statusSD) != 0) {
		return OneWireErrorShortDetected;
	}

	if ((waitRet.status & statusPPD) == 0) {
		return  OneWireErrorNoDevice;

	}

	return 0;
}

WaitReturn OneWireBus::_waitIdle()
{
	WaitReturn ret = {0,0};

	for (int i=0; i<30; i++) {
		delayMicroseconds(100);
		ret.err = i2cTx(NULL, 0, &ret.status, 1);
		if (ret.err) {
			//return ret;
			continue;
		}

		if ((ret.status & status1WB) == 0) {
			return ret;
		}
	}

	ret.err = OneWireErrorTimeout;
	return ret;
}


int OneWireBus::init()
{
	Serial.printf("Initializing 1-wire bus: %s\n", _name);
	select(true);
	uint8_t buf[] = {cmdAdjPort,
		0x00 + 0xf,
		0x20 + 0xf,
		0x40 + 0xf,
		0x60 + 0xf,
		0x80 + 0xf,
	};
	int err = i2cTx(buf, sizeof(buf), NULL, 0);
	select(false);
	return err;
}

SearchReturn OneWireBus::search()
{
	select(true);

	SearchReturn ret = {0,0};

	int discrepancy = -1;
	uint8_t idBytes[8];

	// issue a search command
	uint8_t cmd[] = {cmd1WSearch};

	ret.err = tx(cmd, sizeof(cmd), NULL, 0);

	if (ret.err) {
		goto search_error;
	}

	for (int bit = 0; bit < 64; bit++) {
		// by default, we take the 0 path if we have a discrepency
		uint8_t dir = 0;
		if (bit < _searchLastDiscrepency) {
			// we haven't reached the last discrepancy yet, so we
			// need to repeat the bits of the last device
			dir = (_searchLastDevice >> bit) & 1;
		} else if (bit == _searchLastDiscrepency) {
			// we reached the bit where we picked 0 last time and
			// now we need 1.
			dir = 1;
		}

		// Perform triplet operation
		TripletReturn tripRet = _searchTriplet(dir);

		if (tripRet.err) {
			ret.err = tripRet.err;
			goto search_error;
		}

		if (!tripRet.GotZero && !tripRet.GotOne) {
			ret.err = OneWireErrorDevicesDisappeared;
			goto search_error;
		}


		if (tripRet.GotZero && tripRet.GotOne && !tripRet.Taken) {
			discrepancy = bit;
		}

		ret.device |= uint64_t(tripRet.Taken) << bit;

		if ((bit&7) == 7) {
			idBytes[bit>>3] = ret.device >> (bit-7);
		}
	}

	// Verify the CRC and record the device if we got it right.
	if (!CheckCRC(idBytes, sizeof(idBytes))) {
		ret.err = OneWireErrorCrc;
		goto search_error;
	}

	_searchLastDevice = ret.device;
	_searchLastDiscrepency = discrepancy;
	if (_searchLastDiscrepency == -1) {
		ret.err = OneWireNoMoreDevices;
		_searchLastDevice = 0;
		goto search_done;
	}

search_error:
search_done:
	select(false);
	return ret;
}


int OneWireBus::i2cTx(uint8_t *w, unsigned int wCnt, uint8_t *r, unsigned int rCnt)
{
	int err = 0;

	if (w && wCnt > 0) {
		// send write data
		Wire.beginTransmission(_i2cAddress);

		for (unsigned int i=0; i<wCnt; i++) {
			Wire.write(w[i]);
		}

		err = Wire.endTransmission();

		if (err) {
			Serial.printf("i2c tx error: %i\n", err);
			err = OneWireErrorI2C;
			goto i2ctx_done;
		}
	}

	if (r && rCnt > 0) {
		// read read data
		unsigned int cnt = Wire.requestFrom(_i2cAddress, rCnt);
		if (cnt != rCnt) {
			err = OneWireErrorI2C;
			goto i2ctx_done;
		}

		for (unsigned int i=0; i < rCnt; i++) {
			r[i] = Wire.read();
		}
	}

	err = 0;

i2ctx_done:
	return err;
}

int OneWireBus::txMatch(uint64_t id, uint8_t *w, unsigned int wCnt, uint8_t *r, unsigned int rCnt)
{
	// assume we'll never transmit more than 32 bytes on the wirebus in a transaction
	uint8_t txBuf[32] = {cmd1WMatchRom};

	if (wCnt + 9 > sizeof(txBuf)) {
		Serial.printf("Error, too much data for txMatch, 1st byte: 0x%x\n", w[0]);
		return OneWireErrorUnsupported;
	}

	for (int i=0; i < 8; i++) {
		txBuf[1+i] = uint8_t(id >> (i*8));
	}

	for (unsigned int i=0; i < wCnt; i++) {
		txBuf[9+i] = w[i];
	}

	return tx(txBuf, wCnt + 9, r, rCnt);
}

int OneWireBus::tx(uint8_t *w, unsigned int wCnt, uint8_t *r, unsigned int rCnt)
{
	int err = _reset();

	if (err) {
		return err;
	}

	// send bytes onto 1-wire bus
	for (unsigned int i=0; i<wCnt; i++) {
		uint8_t i2cData[2] = {cmd1WWrite};
		i2cData[1] = w[i];
		err = i2cTx(i2cData, sizeof(i2cData), NULL, 0);

		if (err) {
			goto tx_error;
		}

		WaitReturn waitRet = _waitIdle();
		if (waitRet.err) {
			err = waitRet.err;
			goto tx_error;
		}
	}

	// read bytes from 1-wire bus
	for (unsigned int i=0; i<rCnt; i++) {
		uint8_t i2cData[] = {cmd1WRead};
		err = i2cTx(i2cData, sizeof(i2cData), NULL, 0);
		if (err) {
			goto tx_error;
		}

		WaitReturn waitRet = _waitIdle();
		if (waitRet.err) {
			err = waitRet.err;
			goto tx_error;
		}

		uint8_t i2cData2[] = {cmdSetReadPtr, regRDR};
		err = i2cTx(i2cData2, sizeof(i2cData2), &r[i], 1);
		if (err) {
			goto tx_error;
		}
	}

	err = 0;

tx_error:
	return err;
}


TripletReturn OneWireBus::_searchTriplet(uint8_t direction)
{
	uint8_t dir = 0;
	if (direction != 0) {
		dir = 0x80;
	}

	TripletReturn ret;

	uint8_t i2cData[2] = {cmd1WTriplet};
	i2cData[1] = dir;
	int err = i2cTx(i2cData, sizeof(i2cData), NULL, 0);

	if (err) {
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

const char * OneWireBus::getName()
{
	return _name;
}

void OneWireBus::select(bool enable)
{
	if (enable) {
		digitalWrite(_selectPin, HIGH);
	} else {
		digitalWrite(_selectPin, LOW);
	}
}

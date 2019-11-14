#ifndef ONEWIREBUS_H
#define ONEWIREBUS_H

#include <inttypes.h>

#include "OneWireErrors.h"

enum OneWireFamCode {
    OneWireFamTemp = 0x28,
    OneWireFamAD = 0x26,
    OneWireFamGpio = 0x3A
};

class TripletReturn {
public:
    bool GotZero;
    bool GotOne;
    bool Taken;
    int err;
};

class WaitReturn {
public:
    uint8_t status;
    int err;
};

class SearchReturn {
public:
    uint64_t device;
    int err;
};

class OneWireBus {
    // internal state
    int _searchLastDiscrepency;
    uint64_t _searchLastDevice;

    // constructor params
    const char* _name;
    int _selectPin;
    int _i2cAddress;

    int _reset();
    WaitReturn _waitIdle();
    TripletReturn _searchTriplet(uint8_t direction);

public:
    OneWireBus(const char* name, int selectPin, int i2cAddress);
    // search is designed to be call repeatably and returns devices IDs
    // found. After it hits the last devices, the search is reset internally.
    SearchReturn search();
    const char* getName();
    int init();
    int tx(uint8_t* w, unsigned int wCnt, uint8_t* r, unsigned int rCnt);
    int txMatch(uint64_t id, uint8_t* w, unsigned int wCnt, uint8_t* r, unsigned int rCnt);
    int i2cTx(uint8_t* w, unsigned int wCnt, uint8_t* r, unsigned int rCnt);
    void select(bool enable);
    int matchRom(uint64_t id);
};

#endif

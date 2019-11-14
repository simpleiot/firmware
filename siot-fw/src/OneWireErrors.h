#ifndef ONEWIREERRORS_H
#define ONEWIREERRORS_H

#include <Particle.h>

// errors that may be returned
enum OneWireError {
    OneWireErrorNoDevice = -1,
    OneWireErrorShortDetected = -2,
    OneWireErrorTimeout = -3,
    OneWireErrorDevicesDisappeared = -4,
    OneWireErrorCrc = -5,
    OneWireErrorI2C = -6,
    OneWireErrorUnsupported = -7,
    OneWireErrorWrite = -8,

    // stuff that is not really errors but indicate we are at the end
    // of a list
    OneWireNoMoreDevices = -20,
    OneWireNoMoreData = -21
};

class OneWireErrorCounts {
public:
    int shortDetected;
    int timeout;
    int deviceDisappeared;
    int crc;
    int i2c;
    int unsupported;
    int write;

    OneWireErrorCounts();
    void error(int error);
    String string();
};

const char* OneWireErrorString(int err);

#endif

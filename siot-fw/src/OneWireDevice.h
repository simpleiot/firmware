#ifndef ONEWIREDEVICE_H
#define ONEWIREDEVICE_H

#include <Particle.h>
#include <inttypes.h>

class OneWireDevice {
public:
    int busIndex;
    uint64_t id;
    bool initialized;

    OneWireDevice(int busIndex, uint64_t id);
    bool same(OneWireDevice d);
    uint8_t family();
    String string();
};

#endif

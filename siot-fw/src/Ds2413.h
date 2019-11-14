#ifndef DS2413_H
#define DS2413_H

#include "OneWireBus.h"
#include "Sample.h"

class Ds2413 {
    OneWireBus* _bus;
    uint64_t _id;

public:
    Ds2413(OneWireBus* bus, uint64_t id);
    int init();
    int read(Sample* sample);
    int write(bool, bool);
};

#endif
// Dummy particle.h file for testing logdata.cpp module from gcc
#ifndef __PARTICLE_H
#define __PARTICLE_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cassert>

#include "rng_hal.h"
#include "spark_wiring_string.h"

class Stream {
public:
    inline int available() { return 0; }
    inline int read() { return 0; }
};

#endif /* __PARTICLE_H */

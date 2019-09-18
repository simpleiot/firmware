#include <Particle.h>

#include "print.h"

void print64(uint64_t val)
{
	uint32_t high = uint32_t(val >> 32);
	uint32_t low = uint32_t(val);
	Serial.print(high, HEX);
	Serial.print(low, HEX);
}

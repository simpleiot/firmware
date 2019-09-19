#include <Particle.h>

#include "print.h"

String formatU64Hex(uint64_t val)
{
	return String::format("%08X%08X", uint32_t(val >> 32), uint32_t(val));
}

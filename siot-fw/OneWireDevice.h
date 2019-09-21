#include <Particle.h>
#include <inttypes.h>

class OneWireDevice {
	public:
	uint64_t id;
	int busIndex;

	OneWireDevice(int busIndex, uint64_t id);
	bool same(OneWireDevice d);
	String string();
};

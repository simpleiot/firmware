#ifndef SAMPLE_H
#define SAMPLE_H

#include <Particle.h>
#include <JsonParserGeneratorRK.h>

class Sample
{
	public:
	float value;
	String id;
	String type;

	String string();
	void toJSON(JsonWriter *jw);
};

#endif

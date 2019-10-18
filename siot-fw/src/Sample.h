#ifndef SAMPLE_H
#define SAMPLE_H

#include <JsonParserGeneratorRK.h>
#include <Particle.h>

class Sample {
public:
    float value;
    String id;
    String type;

    String string();
    void toJSON(JsonWriter* jw);
};

#endif

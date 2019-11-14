#include "Particle.h"

#include "DiagnosticsHelperRK.h"

int pubHandler(String data);

void setup() {
	Serial.begin();

	Particle.function("pub", pubHandler);
}

void loop() {

}

int pubHandler(String data) {
	String json = DiagnosticsHelper::getJson();
	Particle.publish("diagData", json, PRIVATE);
	return 0;
}

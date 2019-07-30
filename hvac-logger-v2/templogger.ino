#define ARDUINOJSON_ENABLE_PROGMEM 0
#include <ArduinoJson.h>

void setup() {
  Serial.begin(115200);
  Serial.println("HVAC Logger v2");
}

struct sample {
	String id;
	String s_type;
	float value;
};

// convert a sample struct to JSON. Unused or 0 fields are not populated
String sample_to_json(struct sample s) {
	StaticJsonDocument<200> doc;
	if (s.id.length() > 0) {
		doc["id"] = s.id.c_str();
	}

	if (s.s_type.length() > 0) {
		doc["type"] = s.s_type.c_str();
	}

	if (s.value != 0) {
		doc["value"] = s.value;
	}

	// FIXME, should be able to serialize directly to String
	char buf[100];
	serializeJson(doc, buf, sizeof(buf));
	String ret(buf);
	return ret;
}

float tempSim = 60;

void loop() {
  // We'll leave it on for 1 second...
  delay(1000*60);
  tempSim += 1;
  if (tempSim > 80) {
	  tempSim = 60;
  }

  struct sample s = {
	.id = "12343221",
	.s_type = "temp",
	.value = tempSim
  };

  String data = sample_to_json(s);
  Serial.println(data);
  if (data.length() > 0) {
	  Particle.publish("siot", data, PRIVATE);
  }
}

#define ARDUINOJSON_ENABLE_PROGMEM 0
#include <ArduinoJson.h>

#include "OneWireBus.h"
#include "debug.h"

#define PIN_1_WIRE_UPSTREAM_EN		D18
#define PIN_1_WIRE_DOWNSTREAM_EN	D19

#define PIN_SDA D0
#define PIN_SCL D1

#define I2C_1WIRE_ADDRESS 0x18

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

const unsigned long UPDATE_INTERVAL = 3000;
unsigned long lastUpdate = 0;

OneWireBus oneWireUpstream = OneWireBus(PIN_1_WIRE_UPSTREAM_EN, I2C_1WIRE_ADDRESS);
OneWireBus oneWireDownstream = OneWireBus(PIN_1_WIRE_DOWNSTREAM_EN, I2C_1WIRE_ADDRESS);

void setup() {
  Serial.begin(115200);
  // delay a bit so the first println messages show up on console
  delay(600);
  Serial.println("HVAC Logger v2");
  Serial.println("FW v0.0.3");

  // enable 1-wire drivers
  pinMode(PIN_1_WIRE_DOWNSTREAM_EN, OUTPUT);
  pinMode(PIN_1_WIRE_UPSTREAM_EN, OUTPUT);

  pinMode(PIN_BLACK, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);

  // can only enable one 1-wire bus at a time
  //digitalWrite(PIN_1_WIRE_DOWNSTREAM_EN, HIGH);
  //digitalWrite(PIN_1_WIRE_UPSTREAM_EN, HIGH);

  // enable pullups on i2c lines
  //pinMode(PIN_SDA, INPUT_PULLUP);
  //pinMode(PIN_SCL, INPUT_PULLUP);

  //Wire.setSpeed(CLOCK_SPEED_100KHZ);
  Wire.begin();
}

struct sample {
	String id;
	String s_type;
	float value;
};

void readOneWire() {
	int ret = Wire.requestFrom(I2C_1WIRE_ADDRESS, 1);
	Serial.println("one wire returned: ");
	Serial.println(ret);

	while(Wire.available()){   // slave may send less than requested
		char c = Wire.read();    // receive a byte as character
		Serial.print(c);         // print the character
	}
}

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
	unsigned long currentMillis = millis();

	if (currentMillis - lastUpdate >= UPDATE_INTERVAL)
	{
		lastUpdate = millis();

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

		//readOneWire();

		if (Cellular.ready() && data.length() > 0) {
			Serial.println("publishing data to cloud");
			Particle.publish("siot", data, PRIVATE);
		}

		int err = oneWireDownstream.search();
		if (err) {
			Serial.print("one wire search error: ");
			char *errS = OneWireErrorString(err);
			Serial.println(errS);
		}
	}
}

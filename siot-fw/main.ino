#define ARDUINOJSON_ENABLE_PROGMEM 0
#include <JsonParserGeneratorRK.h>

#include "OneWireManager.h"
#include "debug.h"
#include "print.h"
#include "Sample.h"

#define PIN_1_WIRE_UPSTREAM_EN		D18
#define PIN_1_WIRE_DOWNSTREAM_EN	D19

#define PIN_SDA D0
#define PIN_SCL D1

#define I2C_1WIRE_ADDRESS 0x18

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

OneWireBus oneWireUpstream = OneWireBus("upstream", PIN_1_WIRE_UPSTREAM_EN, I2C_1WIRE_ADDRESS);
OneWireBus oneWireDownstream = OneWireBus("downstream", PIN_1_WIRE_DOWNSTREAM_EN, I2C_1WIRE_ADDRESS);

OneWireManager oneWireManager = OneWireManager();

void setup() {
	Serial.begin(115200);
	// delay a bit so the first println messages show up on console
	delay(600);
	Serial.println("Simple IoT Gateway");
	Serial.println("FW v0.0.4");

	// enable 1-wire drivers
	pinMode(PIN_1_WIRE_DOWNSTREAM_EN, OUTPUT);
	pinMode(PIN_1_WIRE_UPSTREAM_EN, OUTPUT);

	pinMode(PIN_BLACK, OUTPUT);
	pinMode(PIN_GREEN, OUTPUT);

	oneWireManager.addBus(&oneWireUpstream);
	oneWireManager.addBus(&oneWireDownstream);

	Wire.setSpeed(CLOCK_SPEED_400KHZ);
	Wire.begin();

	int err = oneWireManager.init();
	if (err) {
		Serial.printf("Error initializing one wire manager: %s\n",
				OneWireErrorString(err));
	}

	Particle.connect();
}

const unsigned long UPDATE_INTERVAL = 3000;
const unsigned long PUBLISH_INTERVAL = 60*1000;
unsigned long lastUpdate = 0;
unsigned long lastPublish = 0;

void loop() {
	unsigned long currentMillis = millis();

	if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
		bool publish = false;
		lastUpdate = currentMillis;

		oneWireManager.search();

		if ((!lastPublish || currentMillis - lastPublish >= PUBLISH_INTERVAL) &&
		 	Particle.connected()) {
			lastPublish = currentMillis;
			publish = true;
			Serial.println("publishing data to cloud");
		}

		int ret;
		for (int i=0; ; i++) {
			Sample sample;
			ret = oneWireManager.read(&sample);
			if (ret == OneWireNoMoreData) {
				// at end of list
				break;
			} else if (ret) {
				Serial.printf("Warning: read error: %s\n",
						OneWireErrorString(ret));
			} else if (!ret) {
				Serial.printf("sample: %s\n", sample.string().c_str());

				if (publish) {
					JsonWriterStatic<256> jw;
					{
						JsonWriterAutoObject obj(&jw);
						sample.toJSON(&jw);
					}

					Serial.printf("publishing %s\n", jw.getBuffer());
					Particle.publish("sample", jw.getBuffer(),
							PRIVATE);
				}
			}
			if (i >= 100) {
				Serial.println("Warning, read loop is not terminating properly");
				break;
			}
		}


		/*
		Serial.printf("One wire errors:\n%s",
				oneWireManager.getErrors().string().c_str());
		*/

	}
}

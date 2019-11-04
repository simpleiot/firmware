#define ARDUINOJSON_ENABLE_PROGMEM 0
#include "JsonParserGeneratorRK.h"
#include "PublishQueueAsyncRK.h"

#include "OneWireManager.h"
#include "Sample.h"
#include "debug.h"
#include "print.h"

#define PIN_1_WIRE_UPSTREAM_EN D18
#define PIN_1_WIRE_DOWNSTREAM_EN D19

#define PIN_SDA D0
#define PIN_SCL D1

#define I2C_1WIRE_ADDRESS 0x18

#define VERSION 8

PRODUCT_ID(10257);
PRODUCT_VERSION(VERSION);

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

OneWireBus oneWireUpstream = OneWireBus("upstream", PIN_1_WIRE_UPSTREAM_EN, I2C_1WIRE_ADDRESS);
OneWireBus oneWireDownstream = OneWireBus("downstream", PIN_1_WIRE_DOWNSTREAM_EN, I2C_1WIRE_ADDRESS);

OneWireManager oneWireManager = OneWireManager();

retained uint8_t publishQueueRetainedBuffer[2048];
PublishQueueAsync publishQueue(publishQueueRetainedBuffer, sizeof(publishQueueRetainedBuffer));

void setup()
{
    Serial.begin(115200);
    // delay a bit so the first println messages show up on console
    delay(600);
    Serial.println("Simple IoT Gateway");
    Serial.printf("FW v%i\n", VERSION);

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
const unsigned long PUBLISH_INTERVAL = 10 * 1000;
unsigned long lastUpdate = 0 - UPDATE_INTERVAL;
unsigned long lastPublish = 0 - PUBLISH_INTERVAL;

void loop()
{
    unsigned long currentMillis = millis();

    if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
        lastUpdate = currentMillis;

        oneWireManager.search();

        bool publish = false;

        if (currentMillis - lastPublish >= PUBLISH_INTERVAL && Particle.connected()) {
            lastPublish = currentMillis;
            publish = true;
        }

        // read samples
        int ret;

        for (int i = 0;; i++) {
            Sample sample;

            JsonWriterStatic<256> jw;

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
                    jw.startObject();
                    sample.toJSON(&jw);
                    jw.finishObjectOrArray();
                    Serial.printf("publishing %s\n", jw.getBuffer());
                    publishQueue.publish("sample", jw.getBuffer(), PRIVATE, WITH_ACK);
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

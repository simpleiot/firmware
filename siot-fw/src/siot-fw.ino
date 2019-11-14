#define ARDUINOJSON_ENABLE_PROGMEM 0
#include "DiagnosticsHelperRK.h"
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

uint8_t publishQueueRetainedBuffer[2048];
PublishQueueAsync publishQueue(publishQueueRetainedBuffer, sizeof(publishQueueRetainedBuffer));

const BleUuid serviceUuid("5c1b9a0d-b5be-4a40-8f7a-66b36d0a5176");

BleCharacteristic uptimeCharacteristic("uptime", BleCharacteristicProperty::NOTIFY, BleUuid("fdcf4a3f-3fed-4ed2-84e6-04bbb9ae04d4"), serviceUuid);
BleCharacteristic signalStrengthCharacteristic("strength", BleCharacteristicProperty::NOTIFY, BleUuid("cc97c20c-5822-4800-ade5-1f661d2133ee"), serviceUuid);
BleCharacteristic freeMemoryCharacteristic("freeMemory", BleCharacteristicProperty::NOTIFY, BleUuid("d2b26bf3-9792-42fc-9e8a-41f6107df04c"), serviceUuid);

void configureBLE()
{
    BLE.addCharacteristic(uptimeCharacteristic);
    BLE.addCharacteristic(signalStrengthCharacteristic);
    BLE.addCharacteristic(freeMemoryCharacteristic);

    BleAdvertisingData advData;

    // Advertise our private service only
    advData.appendServiceUUID(serviceUuid);

    // Continuously advertise when not connected
    BLE.advertise(&advData);
}

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

    configureBLE();
}

const unsigned long UPDATE_INTERVAL = 5000;
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
                    jw.startArray();
                    jw.startObject();
                    sample.toJSON(&jw);
                    jw.finishObjectOrArray();
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

        if (BLE.connected()) {
            uint8_t uptime = (uint8_t)DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_UPTIME);
            uptimeCharacteristic.setValue(uptime);

            uint8_t signalStrength = (uint8_t)(DiagnosticsHelper::getValue(DIAG_ID_NETWORK_SIGNAL_STRENGTH) >> 8);
            signalStrengthCharacteristic.setValue(signalStrength);

            int32_t usedRAM = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_USED_RAM);
            int32_t totalRAM = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_TOTAL_RAM);
            int32_t freeMem = (totalRAM - usedRAM);
            freeMemoryCharacteristic.setValue(freeMem);
        }

        /*
		Serial.printf("One wire errors:\n%s",
				oneWireManager.getErrors().string().c_str());
		*/
    }
}

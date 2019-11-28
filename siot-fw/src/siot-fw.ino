#define ARDUINOJSON_ENABLE_PROGMEM 0
#include "DiagnosticsHelperRK.h"
#include "JsonParserGeneratorRK.h"
#include "PublishQueueAsyncRK.h"

#include "OneWireManager.h"
#include "Sample.h"
#include "SiotTimer.h"
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

SiotTimer timer = SiotTimer(&oneWireManager);

uint8_t publishQueueRetainedBuffer[2048];
PublishQueueAsync publishQueue(publishQueueRetainedBuffer, sizeof(publishQueueRetainedBuffer));

const BleUuid serviceUuid("5c1b9a0d-b5be-4a40-8f7a-66b36d0a5176");

BleCharacteristic uptimeCharacteristic("uptime", BleCharacteristicProperty::NOTIFY, BleUuid("fdcf0000-3fed-4ed2-84e6-04bbb9ae04d4"), serviceUuid);
BleCharacteristic signalStrengthCharacteristic("strength", BleCharacteristicProperty::NOTIFY, BleUuid("fdcf0001-3fed-4ed2-84e6-04bbb9ae04d4"), serviceUuid);
BleCharacteristic freeMemoryCharacteristic("freeMemory", BleCharacteristicProperty::NOTIFY, BleUuid("fdcf0002-3fed-4ed2-84e6-04bbb9ae04d4"), serviceUuid);
BleCharacteristic modelCharacteristic("model", BleCharacteristicProperty::READ, BleUuid("fdcf0003-3fed-4ed2-84e6-04bbb9ae04d4"), serviceUuid);
BleCharacteristic wifiSSIDCharacteristic("wifiSSID", BleCharacteristicProperty::READ, BleUuid("fdcf0004-3fed-4ed2-84e6-04bbb9ae04d4"), serviceUuid);
BleCharacteristic connectedCharacteristic("connected", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::READ, BleUuid("fdcf0005-3fed-4ed2-84e6-04bbb9ae04d4"), serviceUuid);

String setSSID;

static void onSetWifiSSID(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context)
{
    char buf[100];
    if (len > sizeof(buf)) {
        Serial.println("ssid is too long");
        return;
    }

    strncpy(buf, (char*)data, len);
    buf[len] = 0;
    Serial.printf("set wifi SSID: %s\n", buf);
    setSSID = buf;
}

static void onSetWifiPass(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context)
{
#if Wiring_WiFi
    char buf[100];
    if (len > sizeof(buf)) {
        Serial.println("ssid is too long");
        return;
    }

    strncpy(buf, (char*)data, len);
    buf[len] = 0;
    Serial.printf("set wifi pass: %s\n", buf);
    if (PLATFORM_ID == PLATFORM_ARGON) {
        WiFi.disconnect();
        if (!WiFi.clearCredentials()) {
            Serial.println("Failed to clear Wifi credentials");
        }
        if (!WiFi.setCredentials(setSSID.c_str(), buf)) {
            Serial.println("Failed to set Wifi credentials");
        } else {
            Serial.println("WiFi credentials set");
            wifiSSIDCharacteristic.setValue(setSSID);
            // so far, the only way I can get new wifi settings to stick is reset the system
            /*
            WiFi.off();
            WiFi.on();
            Particle.connect();
            */
            System.reset();
        }
    }
#endif
}

extern BleCharacteristic timerFireDurationCharacteristic;
extern BleCharacteristic timerFireTimeCharacteristic;

static void onTimerFireDuration(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context)
{
    if (len < 4) {
        Serial.printf("not enough data to fill int: %i\n", len);
        return;
    }

    uint32_t timerDuration = *(uint32_t*)data;
    timer.setFireDuration(timerDuration);
    Serial.printf("timerDuration set to: %i\n", timerDuration);
    timerFireDurationCharacteristic.setValue(inet_htonl(timerDuration));
}

static void onTimerFireTime(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context)
{
    if (len < 4) {
        Serial.printf("not enough data to fill int: %i\n", len);
        return;
    }

    uint32_t time = *(uint32_t*)data;
    timer.setFireTime(time);
    Serial.printf("timer fire time set to: %i\n", time);
    timerFireTimeCharacteristic.setValue(inet_htonl(time));
}

static void onTimerFire(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context)
{
    Serial.printf("Timer fire\n");
    timer.fire();
}

static void onSetTime(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context)
{
    uint32_t time = *(uint32_t*)data;
    Serial.printf("set time: %i\n", time);
    Time.setTime(time);
}

BleCharacteristic setWifiSSIDCharacteristic("setWifiPass", BleCharacteristicProperty::WRITE_WO_RSP, BleUuid("fdcf0006-3fed-4ed2-84e6-04bbb9ae04d4"), serviceUuid, onSetWifiSSID, NULL);
BleCharacteristic setWifiPassCharacteristic("setWifiPass", BleCharacteristicProperty::WRITE_WO_RSP, BleUuid("fdcf0007-3fed-4ed2-84e6-04bbb9ae04d4"), serviceUuid, onSetWifiPass, NULL);
BleCharacteristic deviceNameCharacteristic("deviceName", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, BleUuid("fdcf0008-3fed-4ed2-84e6-04bbb9ae04d4"), serviceUuid, onSetWifiPass, NULL);
BleCharacteristic timerFireDurationCharacteristic("timerFireDuration", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, BleUuid("fdcf0009-3fed-4ed2-84e6-04bbb9ae04d4"), serviceUuid, onTimerFireDuration, NULL);
BleCharacteristic timerFireCharacteristic("timerFire", BleCharacteristicProperty::WRITE_WO_RSP, BleUuid("fdcf000a-3fed-4ed2-84e6-04bbb9ae04d4"), serviceUuid, onTimerFire, NULL);
BleCharacteristic currentTimeCharacteristic("currentTime", BleCharacteristicProperty::NOTIFY | BleCharacteristicProperty::WRITE_WO_RSP, BleUuid("fdcf000b-3fed-4ed2-84e6-04bbb9ae04d4"), serviceUuid, onSetTime, NULL);
BleCharacteristic timerFireTimeCharacteristic("timerFireTime", BleCharacteristicProperty::WRITE_WO_RSP | BleCharacteristicProperty::READ, BleUuid("fdcf000c-3fed-4ed2-84e6-04bbb9ae04d4"), serviceUuid, onTimerFireTime, NULL);

void configureBLE()
{
    BLE.addCharacteristic(uptimeCharacteristic);
    BLE.addCharacteristic(signalStrengthCharacteristic);
    BLE.addCharacteristic(freeMemoryCharacteristic);
    BLE.addCharacteristic(modelCharacteristic);
    BLE.addCharacteristic(connectedCharacteristic);
    BLE.addCharacteristic(wifiSSIDCharacteristic);
    BLE.addCharacteristic(setWifiSSIDCharacteristic);
    BLE.addCharacteristic(setWifiPassCharacteristic);
    BLE.addCharacteristic(deviceNameCharacteristic);
    BLE.addCharacteristic(timerFireDurationCharacteristic);
    BLE.addCharacteristic(timerFireCharacteristic);
    BLE.addCharacteristic(currentTimeCharacteristic);
    BLE.addCharacteristic(timerFireTimeCharacteristic);

    BleAdvertisingData advData;

    // Advertise our private service only
    advData.appendServiceUUID(serviceUuid);

    // Continuously advertise when not connected
    BLE.advertise(&advData);

    connectedCharacteristic.setValue(Particle.connected());
    timerFireDurationCharacteristic.setValue(0);
    timerFireTimeCharacteristic.setValue(0);

    currentTimeCharacteristic.setValue(Time.now());
}

const unsigned long UPDATE_INTERVAL = 10000;
const unsigned long TIME_UPDATE_INTERVAL = 1000;
unsigned long PUBLISH_INTERVAL = 20 * 1000;
unsigned long lastUpdate = 0 - UPDATE_INTERVAL;
unsigned long lastTimeUpdate = 0 - TIME_UPDATE_INTERVAL;
unsigned long lastPublish = 0 - PUBLISH_INTERVAL;

#define PUBLISH_INTERVAL_CELL 5 * 60 * 1000;

void deviceNameHandler(const char* topic, const char* data)
{
    Serial.printf("device name received: %s\n", data);
    deviceNameCharacteristic.setValue(data);
}

void setup()
{
    Serial.begin(115200);
    // delay a bit so the first println messages show up on console
    delay(600);
    Serial.println("Simple IoT Gateway");
    Serial.printf("FW v%i\n", VERSION);

    configureBLE();

    // this does not work yet ...
    Particle.subscribe("particle/device/name", deviceNameHandler, MY_DEVICES);
    Particle.publish("particle/device/name", PRIVATE);

    switch (PLATFORM_ID) {
    case PLATFORM_ARGON: {
        Serial.println("Running on Argon");
        modelCharacteristic.setValue("Argon");
#if Wiring_WiFi
        WiFiAccessPoint ap[1];
        int found = WiFi.getCredentials(ap, 1);
        if (found == 1) {
            wifiSSIDCharacteristic.setValue(ap[0].ssid);
        }
#endif
        break;
    }
    case PLATFORM_BORON:
        Serial.println("Running on Boron");
        PUBLISH_INTERVAL = PUBLISH_INTERVAL_CELL;
        modelCharacteristic.setValue("Boron");
        break;
    case PLATFORM_XENON:
        Serial.println("Running on Xenon");
        modelCharacteristic.setValue("Xenon");
        break;
    case PLATFORM_ELECTRON_PRODUCTION:
        Serial.println("Running on Electron");
        PUBLISH_INTERVAL = PUBLISH_INTERVAL_CELL;
        break;
    default:
        Serial.printf("Unknown platform: %i\n", PLATFORM_ID);
        break;
    }

    Serial.printf("Publish interval: %is\n", PUBLISH_INTERVAL / 1000);

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

bool connected = false;

void loop()
{
    unsigned long currentMillis = millis();

    if (currentMillis - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
        uint32_t now = inet_htonl(Time.now());
        currentTimeCharacteristic.setValue(now);
        lastTimeUpdate = currentMillis;
        timer.run();
    }

    if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
        lastUpdate = currentMillis;
        bool conn = Particle.connected();

        oneWireManager.search();

        bool publish = false;

        if (currentMillis - lastPublish >= PUBLISH_INTERVAL && conn) {
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
                    publishQueue.publish("sample", jw.getBuffer(), PRIVATE, NO_ACK);
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
            freeMemoryCharacteristic.setValue(inet_htonl(freeMem));

            if (conn != connected) {
                connected = conn;
                connectedCharacteristic.setValue(conn);
            }
        }

        /*
		Serial.printf("One wire errors:\n%s",
				oneWireManager.getErrors().string().c_str());
		*/
    }
}

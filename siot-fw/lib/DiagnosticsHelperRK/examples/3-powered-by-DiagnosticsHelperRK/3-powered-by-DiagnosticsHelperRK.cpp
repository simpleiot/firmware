#include "Particle.h"

#include "DiagnosticsHelperRK.h"

SerialLogHandler logHandler;

unsigned long lastPublish = 0;
int lastPowerSource = -1;

void setup() {
	Serial.begin();
}

void loop() {

	int powerSource = DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_POWER_SOURCE);
	if (powerSource != lastPowerSource) {
		if (millis() - lastPublish >= 1000 && Particle.connected()) {
			lastPublish = millis();

		    // POWER_SOURCE_UNKNOWN = 0,
			// POWER_SOURCE_VIN = 1,
			// POWER_SOURCE_USB_HOST = 2,
			// POWER_SOURCE_USB_ADAPTER = 3,
			// POWER_SOURCE_USB_OTG = 4,
			// POWER_SOURCE_BATTERY = 5

			char buf[128];
			snprintf(buf, sizeof(buf), "powerSource=%d", powerSource);

			Particle.publish("powerSource", buf, PRIVATE);
			Log.info(buf);

			lastPowerSource = powerSource;
		}
	}

}

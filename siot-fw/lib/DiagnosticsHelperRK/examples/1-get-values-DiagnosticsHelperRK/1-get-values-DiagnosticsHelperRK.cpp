#include "Particle.h"

#include "DiagnosticsHelperRK.h"

SerialLogHandler logHandler;

const unsigned long CHECK_INTERVAL_MS = 5000;
unsigned long lastCheck = 0;

void setup() {
	Serial.begin();
}

void loop() {
	if (millis() - lastCheck >= CHECK_INTERVAL_MS) {
		lastCheck = millis();

		// Constants for the various diagnostics items can be found here:
		// https://github.com/particle-iot/firmware/blob/develop/services/inc/diagnostics.h

		Log.info("cloud:stat=%d", DiagnosticsHelper::getValue(DIAG_ID_CLOUD_CONNECTION_STATUS));
		Log.info("cloud:err=%d", DiagnosticsHelper::getValue(DIAG_ID_CLOUD_CONNECTION_ERROR_CODE));
		Log.info("cloud:dconn=%d", DiagnosticsHelper::getValue(DIAG_ID_CLOUD_DISCONNECTS));
		Log.info("cloud:connatt=%d", DiagnosticsHelper::getValue(DIAG_ID_CLOUD_CONNECTION_ATTEMPTS));
		Log.info("cloud:dconnrsn=%d", DiagnosticsHelper::getValue(DIAG_ID_CLOUD_DISCONNECTION_REASON));
	}
}

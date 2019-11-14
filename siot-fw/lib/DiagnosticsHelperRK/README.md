# Diagnostics Helper

- Location: [https://github.com/rickkas7/DiagnosticsHelperRK](https://github.com/rickkas7/DiagnosticsHelperRK)
- License: MIT

The system diagnostics feature in Particle Device OS 0.8.0 can make troubleshooting device problems easier.

For example, when viewing a device in [the console](https://console.particle.io), you can view the device vitals:

![Device Vitals](images/devicevitals.png)

This is periodically updated automatically so you don't have to worry about it. You can also use the cloud API to [request a refresh of the device vitals](https://docs.particle.io/reference/api/#refresh-device-vitals).

You can also monitor the [device vitals event](https://docs.particle.io/reference/api/#device-vitals-event) by subscribing to it.

But what if you want to get at some of this information from your user firmware, so you can send it somewhere else, or use it to control things in your firmware?

For example, the [Device Key Helper](https://github.com/rickkas7/DeviceKeyHelperRK) library uses diagnostics to find the last connection error. From this, it can determine if the connection failed because of a keys error and take appropriate action.

Some things like free memory are more easily obtained using the regular system firmware calls like [System.freeMemory](https://docs.particle.io/reference/firmware/#freememory-), but others are only available in diagnostics.

## Diagnostic Items

The various items that are available are described in the [device vitals metadata](https://docs.particle.io/reference/api/#get-device-vitals-metadata). 

There's also [the official list in the Device OS source](https://github.com/particle-iot/firmware/blob/develop/services/inc/diagnostics.h):

```
typedef enum diag_id {
    DIAG_ID_INVALID = 0, // Invalid source ID
    DIAG_ID_SYSTEM_LAST_RESET_REASON = 1, // sys:reset
    DIAG_ID_SYSTEM_FREE_MEMORY = 2, // mem:free
    DIAG_ID_SYSTEM_BATTERY_CHARGE = 3, // batt:soc
    DIAG_ID_SYSTEM_SYSTEM_LOOPS = 4, // sys:loops
    DIAG_ID_SYSTEM_APPLICATION_LOOPS = 5, // app:loops
    DIAG_ID_SYSTEM_UPTIME = 6, // sys:uptime
    DIAG_ID_SYSTEM_BATTERY_STATE = 7, // batt:state
    DIAG_ID_SYSTEM_POWER_SOURCE = 24, // pwr::src
    DIAG_ID_NETWORK_CONNECTION_STATUS = 8, // net:stat
    DIAG_ID_NETWORK_CONNECTION_ERROR_CODE = 9, // net:err
    DIAG_ID_NETWORK_DISCONNECTS = 12, // net:dconn
    DIAG_ID_NETWORK_CONNECTION_ATTEMPTS = 27, // net:connatt
    DIAG_ID_NETWORK_DISCONNECTION_REASON = 28, // net:dconnrsn
    DIAG_ID_NETWORK_IPV4_ADDRESS = 15, // net:ip:addr
    DIAG_ID_NETWORK_IPV4_GATEWAY = 16, // net.ip:gw
    DIAG_ID_NETWORK_FLAGS = 17, // net:flags
    DIAG_ID_NETWORK_COUNTRY_CODE = 18, // net:cntry
    DIAG_ID_NETWORK_RSSI = 19, // net:rssi
    DIAG_ID_NETWORK_SIGNAL_STRENGTH_VALUE = 37, // net:sigstrv
    DIAG_ID_NETWORK_SIGNAL_STRENGTH = 33, // net:sigstr
    DIAG_ID_NETWORK_SIGNAL_QUALITY = 34, // net:sigqual
    DIAG_ID_NETWORK_SIGNAL_QUALITY_VALUE = 35, // net:sigqualv
    DIAG_ID_NETWORK_ACCESS_TECNHOLOGY = 36, // net:at
    DIAG_ID_CLOUD_CONNECTION_STATUS = 10, // cloud:stat
    DIAG_ID_CLOUD_CONNECTION_ERROR_CODE = 13, // cloud:err
    DIAG_ID_CLOUD_DISCONNECTS = 14, // cloud:dconn
    DIAG_ID_CLOUD_CONNECTION_ATTEMPTS = 29, // cloud:connatt
    DIAG_ID_CLOUD_DISCONNECTION_REASON = 30, // cloud:dconnrsn
    DIAG_ID_CLOUD_REPEATED_MESSAGES = 21, // coap:resend
    DIAG_ID_CLOUD_UNACKNOWLEDGED_MESSAGES = 22, // coap:unack
    DIAG_ID_CLOUD_RATE_LIMITED_EVENTS = 20, // pub:throttle
    DIAG_ID_SYSTEM_TOTAL_RAM = 25, // sys:tram
    DIAG_ID_SYSTEM_USED_RAM = 26, // sys:uram
    DIAG_ID_USER = 32768 // Base value for application-specific source IDs
} diag_id;
```

## Diagnostic Helper Functions

There are only two functions in the DiagnosticHelperRK library:

### getValue

Gets a single diagnostic value.

```
int32_t DiagnosticHelper::getValue(uint16_t id);
```

The id is the code above, such as `DIAG_ID_CLOUD_CONNECTION_ERROR_CODE`.

You might use it like this:

```
Log.info("cloud:err=%d", DiagnosticsHelper::getValue(DIAG_ID_CLOUD_CONNECTION_ERROR_CODE));
```

### getJson

Gets the diagnostic data as a JSON object in a String.

```
String DiagnosticHelper::getJson();
```

For example, you might get something like this on a Photon:

```
{"sys:uptime":2001,"net:stat":4,"net:err":0,"cloud:stat":2,"net:dconn":0,"cloud:err":0,"cloud:dconn":0,"net:rssi":-18688,"pub:limit":0,"coap:unack":0,"sys:tram":83200,"sys:uram":32992,"net:connatt":1,"net:dconnrsn":0,"cloud:connatt":1,"cloud:dconnrsn":0,"net:sigstr":13823,"net:sigqual":8257,"net:sigqualv":1245184,"net:at":1,"net:sigstrv":-4784128,"_":""}
```

## Examples

### 1-get-values

```
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
```

Running this, you'd see something like this on the debug serial:

```
0000010000 [app] INFO: cloud:stat=2
0000010000 [app] INFO: cloud:err=0
0000010000 [app] INFO: cloud:dconn=0
0000010000 [app] INFO: cloud:connatt=1
0000010000 [app] INFO: cloud:dconnrsn=0
```


### 2-get-json

This example gets the JSON data and publishes it when the "pub" function is called.

```
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
```

You might see something like this in the event log:

![Event Log](images/event.png)



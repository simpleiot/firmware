#ifndef __DIAGNOSTICSHELPERRK_H
#define __DIAGNOSTICSHELPERRK_H

// Location: https://github.com/rickkas7/DiagnosticsHelperRK
// License: MIT

#include "Particle.h"

#if SYSTEM_VERSION < 0x00080000
#error system firmware 0.8.0 or later is required for this library
#endif

#if PLATFORM_ID == 0
#error diagnostics are not supported on the Spark Core
#endif

/**
 * @brief Handy functions for getting system diagnostic data from user firmware
 */
class DiagnosticsHelper {
public:
	/**
	 * @brief Get a single diagnostic value
	 *
	 * @param id The diagnostic ID to get. For example: DIAG_ID_CLOUD_DISCONNECTION_REASON (30).
	 *
	 * @return The value. It is a signed 32-bit integer.
	 *
	 * The list of ids that you can query can be found here:
	 * https://github.com/particle-iot/firmware/blob/develop/services/inc/diagnostics.h
	 *
	 */
	static int32_t getValue(uint16_t id);

	/**
	 * @brief Get a String containing a JSON object
	 *
	 * @return A String object containing a JSON object with diagnostic data.
	 *
	 * This object is not the same as the system diagnostics event. The data is flat, with a
	 * single object with hierarchical names (like sys:uptime) instead of embedded objects
	 * in the diagnostics event.
	 *
	 * The mapping should be pretty obvious and the fields are described here:
	 * https://docs.particle.io/reference/api/#device-vitals-event
	 *
	 * If there is an abbreviation you're not sure about, the list here expands the short
	 * names in the JSON object keys:
	 * https://github.com/particle-iot/firmware/blob/develop/services/inc/diagnostics.h
	 */
	static String getJson();
};

#endif /* __DIAGNOSTICSHELPERRK_H */

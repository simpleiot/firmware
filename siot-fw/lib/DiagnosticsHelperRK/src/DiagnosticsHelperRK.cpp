#include "DiagnosticsHelperRK.h"

// Location: https://github.com/rickkas7/DiagnosticsHelperRK
// License: MIT

// [static]
int32_t DiagnosticsHelper::getValue(uint16_t id) {
    // Only available in system firmware 0.8.0 and later!
	struct Data {
		union {
			struct __attribute__((packed)) {
				uint16_t 	idSize;
				uint16_t 	valueSize;
				uint16_t 	id;
				int32_t     value;
			} d;
			uint8_t b[10];
		} u;
		size_t offset;
	};
    Data data;
    data.offset = data.u.d.value = 0;

    struct {
        static bool appender(void* appender, const uint8_t* data, size_t size) {
            Data *d = (Data *)appender;
            if ((d->offset + size) <= sizeof(Data::u)) {
                memcpy(&d->u.b[d->offset], data, size);
                d->offset += size;
            }
            return true;
        }
    } Callback;

    system_format_diag_data(&id, 1, 1, Callback.appender, &data, nullptr);

    // Log.info("idSize=%u valueSize=%u id=%u value=%ld", data.u.d.idSize, data.u.d.valueSize, data.u.d.id, data.u.d.value);

    if (data.offset == sizeof(Data::u)) {
    	return data.u.d.value;
    }
    else {
        return 0;
    }
}



// [static]
String DiagnosticsHelper::getJson() {
	String result;

	result.reserve(256);

    struct {
        static bool appender(void* appender, const uint8_t* data, size_t size) {
            String *s = (String *)appender;
            return (bool) s->concat(String((const char *)data, size));
        }
    } Callback;

    system_format_diag_data(nullptr, 0, 0, Callback.appender, &result, nullptr);


	return result;
}

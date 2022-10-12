// Warning, the following will not run on UNO/AVR devices -- not enough memory
// tested on a SAMD21 device

#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <pb.h>

#include <siot_serial.pb.h>
#include <point.pb.h>
#include <timestamp.pb.h>
#include <stdarg.h>

#include <PacketSerial.h>

PacketSerial cobsWrapper;

// NOTE: maximum receive buffer length in Uno default serial ISR is 64 bytes.
uint8_t buffer[256];

// kermit
uint16_t CRC16K(uint8_t *x, uint8_t len)
{
    uint8_t *data = x;
    uint16_t crc = 0;

    while (len--)
    {
        crc ^= *data++;
        for (uint8_t k = 0; k < 8; k++)
            if (crc & 1)
                crc = (crc >> 1) ^ 0x8408;
            else
                crc = crc >> 1;
    }

    return crc;
}

bool send_message(siot_Serial *msg)
{
    static uint8_t pb_msg_counter = 0;
    uint16_t crc1 = 0;
    int status;
    memset(buffer, 0x00, sizeof(buffer));

    pb_ostream_t stream = pb_ostream_from_buffer(&(buffer[1]), sizeof(buffer) - 3);

    status = pb_encode(&stream, siot_Serial_fields, msg);

    if (!status) {
      return false;
    }

    buffer[0] = pb_msg_counter;

    crc1 = CRC16K(buffer, (stream.bytes_written + 1));

    buffer[stream.bytes_written + 1] = (uint8_t)((crc1 & 0x00FF));      // CRC Placeholder
    buffer[stream.bytes_written + 2] = (uint8_t)((crc1 & 0xFF00) >> 8); // CRC Placeholder

    pb_msg_counter++;

    cobsWrapper.send(buffer, stream.bytes_written + 3);
    cobsWrapper.update();

    return true;
}

// format and send ascii message with cobs
void cprintf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf((char*)buffer, fmt, args);
	va_end(args);
	cobsWrapper.send(buffer, strlen((char*)buffer));
	cobsWrapper.update();
}

unsigned long lastNormalData;

// the setup function runs once when you press reset or power the board
void setup()
{
    Serial.begin(115200);

    cobsWrapper.setStream(&Serial);

    cprintf("Starting COBS wrapped PB test.");

    lastNormalData = millis();
}

// sim values
int count = 0;
float temp1 = 23.0;
bool temp1Up = false;
float temp2 = 19.0;
bool temp2Up = false;

void update_sim_values() {
	if (temp1Up) {
		temp1 += 0.2;
		if (temp1 > 50) {
			temp1Up = false;
		}
	} else {
		temp1 -= 0.2;
		if (temp1 < 20) {
			temp1Up = true;
		}
	}

	if (temp2Up) {
		temp2 += 0.2;
		if (temp2 > 50) {
			temp2Up = false;
		}
	} else {
		temp2 -= 0.2;
		if (temp2 < 20) {
			temp2Up = true;
		}
	}
}

// high rate sim values
float voltage = 120;
bool voltageUp = false;

void update_hr_sim_values() {
	if (voltageUp) {
		voltage += 0.2;
		if (voltage > 277) {
			voltageUp = false;
		}
	} else {
		voltage  -= 0.2;
		if (voltage < 110) {
			voltageUp = true;
		}
	}
}


void loop()
{
	unsigned long now = millis();
	if ((now - lastNormalData) > 1000) {
		// send low rate data
		lastNormalData += 1000;
		cprintf("Loop %d", count);

		siot_Serial msg = siot_Serial_init_default;
		msg.points_count = 2;

		// has_time must be set to true, or we'll get a decode error at the other end
		msg.points[0].has_time = true;
		strcpy(msg.points[0].type, "temp");
		strcpy(msg.points[0].key, "front");
		msg.points[0].value = temp1;
		msg.points[0].index = 0;

		msg.points[1].has_time = true;
		strcpy(msg.points[1].type, "temp");
		strcpy(msg.points[1].key, "back");
		msg.points[1].value = temp2;
		msg.points[1].index = 1;

		if (!send_message(&msg)) {
			cprintf("Encoding failed");
		}

		update_sim_values();
	}

	// send high rate data
	siot_Serial msg = siot_Serial_init_default;
	strcpy(msg.subject, "phr");
	msg.points_count = 1;

	msg.points[0].has_time = true;
	strcpy(msg.points[0].type, "voltage");
	msg.points[0].value = voltage;

	if (!send_message(&msg)) {
		cprintf("Encoding failed");
	}

	update_hr_sim_values();

	count++;
}

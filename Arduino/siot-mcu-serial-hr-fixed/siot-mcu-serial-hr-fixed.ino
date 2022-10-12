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
uint8_t bufferhr[256];
int bufferhrLen;

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
	uint16_t crc1 = 0;
	Serial.begin(115200);

	cobsWrapper.setStream(&Serial);

	cprintf("Starting COBS wrapped PB test.");

	lastNormalData = millis();

	// send high rate data
	siot_Serial msg = siot_Serial_init_default;
	strcpy(msg.subject, "phr");
	msg.points_count = 1;

	msg.points[0].has_time = true;
	strcpy(msg.points[0].type, "voltage");
	msg.points[0].value = 260.5;

	memset(bufferhr, 0x00, sizeof(bufferhr));

	pb_ostream_t stream = pb_ostream_from_buffer(&(bufferhr[1]), sizeof(bufferhr) - 3);

	bool status = pb_encode(&stream, siot_Serial_fields, &msg);

	if (!status) {
		cprintf("Error encoding static message buffer");
	}

	bufferhr[0] = 23;

	crc1 = CRC16K(bufferhr, (stream.bytes_written + 1));

	bufferhr[stream.bytes_written + 1] = (uint8_t)((crc1 & 0x00FF));      // CRC Placeholder
	bufferhr[stream.bytes_written + 2] = (uint8_t)((crc1 & 0xFF00) >> 8); // CRC Placeholder

	bufferhrLen = stream.bytes_written + 3;
}

int count = 0;

void loop()
{
	unsigned long now = millis();
	if ((now - lastNormalData) > 1000) {
		// send low rate data
		lastNormalData += 1000;
		cprintf("Loop %d", count);
	}

	cobsWrapper.send(bufferhr, bufferhrLen);
	cobsWrapper.update();

	count++;
}

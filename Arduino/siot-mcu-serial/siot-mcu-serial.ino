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

// define delay between consecutive sending of packets
// note, if you set this to 0, and don't set HR_PACKETS, you may overrun the SIOT host.
int DELAY_BETWEEN_PACKETS_MS = 500;
// The following specifies the message to be a "high rate" message which
// tells the SIOT host to not process it as a normal point, but send
// it to high-rate message subjects which are processed by clients
// expecting high rate points. See https://docs.simpleiot.org/docs/ref/api.html
int HR_PACKETS = 0;
// define the number of points to send in each message
// FIXME, for some reason we can't go over 10 for POINT_COUNT -- not sure yet
// what is going on there
int POINT_COUNT = 10;

PacketSerial cobsWrapper;

// NOTE: maximum receive buffer length in Uno default serial ISR is 64 bytes.
uint8_t buffer[1000];

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

int pb_encoding_time;
int crc_time;
int cobs_send_time;

bool send_message(siot_Serial *msg)
{
    static uint8_t pb_msg_counter = 0;
    uint16_t crc1 = 0;
    int status;
    int start = micros();
    memset(buffer, 0x00, sizeof(buffer));

    pb_ostream_t stream = pb_ostream_from_buffer(&(buffer[1]), sizeof(buffer) - 3);

    status = pb_encode(&stream, siot_Serial_fields, msg);

    if (!status) {
      return false;
    }

    pb_encoding_time = micros() - start;
    start = micros();

    buffer[0] = pb_msg_counter;

    crc1 = CRC16K(buffer, (stream.bytes_written + 1));
    crc_time = micros() - start;
    start = micros();

    buffer[stream.bytes_written + 1] = (uint8_t)((crc1 & 0x00FF));      // CRC Placeholder
    buffer[stream.bytes_written + 2] = (uint8_t)((crc1 & 0xFF00) >> 8); // CRC Placeholder

    pb_msg_counter++;

    cobsWrapper.send(buffer, stream.bytes_written + 3);
    cobsWrapper.update();

    cobs_send_time = micros() - start;

    return true;
}

int pb_encoding_time_hr;
int crc_time_hr;
int cobs_send_time_hr;

bool send_hr_message(PointArray *msg)
{
    static uint8_t pb_msg_counter = 0;
    uint16_t crc1 = 0;
    int status;
    int start = micros();
    //memset(buffer, 0x00, sizeof(buffer));

    pb_ostream_t stream = pb_ostream_from_buffer(&(buffer[1]), sizeof(buffer) - 3);

    status = pb_encode(&stream, PointArray_fields, msg);

    pb_encoding_time_hr = micros() - start;

    if (!status) {
      return false;
    }


    /*
    start = micros();

    buffer[0] = pb_msg_counter;

    crc1 = CRC16K(buffer, (stream.bytes_written + 1));
    crc_time_hr = micros() - start;
    start = micros();

    buffer[stream.bytes_written + 1] = (uint8_t)((crc1 & 0x00FF));      // CRC Placeholder
    buffer[stream.bytes_written + 2] = (uint8_t)((crc1 & 0xFF00) >> 8); // CRC Placeholder

    pb_msg_counter++;

    cobsWrapper.send(buffer, stream.bytes_written + 3);
    cobsWrapper.update();

    cobs_send_time_hr = micros() - start;
    */

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

// the setup function runs once when you press reset or power the board
void setup()
{
    Serial.begin(115200);

    cobsWrapper.setStream(&Serial);

    cprintf("Starting COBS wrapped PB test.");
}

int count = 0;

siot_Serial msg = siot_Serial_init_default;
PointArray hr_pts = PointArray_init_default;

void loop()
{
    cprintf("Loop %d", count);

    hr_pts.values_count = 200;
    if (!send_hr_message(&hr_pts)) {
        cprintf("HR Encoding failed");
    }

    if (HR_PACKETS) {
	strcpy(msg.subject, "phr");
    }
    msg.points_count = POINT_COUNT;

    // has_time must be set to true, or we'll get a decode error at the other end
    strcpy(msg.points[0].type, "metric");
    strcpy(msg.points[0].key, "pb-encode");
    msg.points[0].value = pb_encoding_time;

    strcpy(msg.points[1].type, "temp");
    strcpy(msg.points[1].key, "back");
    msg.points[1].value = 19.4;
    msg.points[1].index = 1;

    strcpy(msg.points[2].type, "voltage");
    msg.points[2].value = 277;

    strcpy(msg.points[3].type, "temp");
    strcpy(msg.points[3].key, "front");
    msg.points[3].value = 23.3;
    msg.points[3].index = 0;

    strcpy(msg.points[4].type, "metric");
    strcpy(msg.points[4].key, "crc");
    msg.points[4].value = crc_time;

    strcpy(msg.points[5].type, "metric");
    strcpy(msg.points[5].key, "cobs");
    msg.points[5].value = cobs_send_time;

    strcpy(msg.points[6].type, "metric");
    strcpy(msg.points[6].key, "pb-encode-hr");
    msg.points[6].value = pb_encoding_time_hr;

    for (int i=7; i<POINT_COUNT; i++) {
	    strcpy(msg.points[i].type, "testPoint");
	    msg.points[i].value = i*2;
	    msg.points[i].index = i-7;
    }

    if (!send_message(&msg)) {
        cprintf("Encoding failed");
    }

    count++;



    if (DELAY_BETWEEN_PACKETS_MS > 0) {
    	delay(DELAY_BETWEEN_PACKETS_MS);
    }
}

#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <pb.h>

#include <siot_serial.pb.h>
#include <point.pb.h>
#include <timestamp.pb.h>

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

void cobs_print(char *print_string)
{
    cobsWrapper.send(print_string, strlen(print_string));
    cobsWrapper.update();
}

// the setup function runs once when you press reset or power the board
void setup()
{
    Serial.begin(115200);

    cobsWrapper.setStream(&Serial);

    sprintf(buffer, "Starting COBS wrapped PB test.\r\n");
    cobs_print(buffer);
}

int count = 0;

void loop()
{
    static uint16_t msg_counter = 0;

    sprintf(buffer, "Loop %d", count);
    cobs_print(buffer);
    delay(1000);

    count++;

    siot_Serial msg = siot_Serial_init_default;
    msg.points_count = 0;
    msg.points[0].has_time = true;
    strcpy(msg.points[0].type, "temp");
    msg.points[0].value = 23.3;

    if (!send_message(&msg)) {
        sprintf(buffer, "Encoding failed");
        cobs_print(buffer);
    }

    delay(1000);
}

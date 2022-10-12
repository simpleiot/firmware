#include <stdio.h>
#include <pb.h>
#include <pb_common.h>
#include <pb_encode.h>
#include <siot_serial.pb.h>
#include <point.pb.h>
#include <timestamp.pb.h>

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

void print_hex(void *buf, int len)
{
	char *bufc = buf;

	for (int i=0; i < len; i++) {
		printf("%02x ", bufc[i]);
	}

	printf("\n");
}

void main()
{
	printf("pb-test\n");

	siot_Serial msg;
	memset(&msg, 0, sizeof(msg));
	msg.points_count = 1;
	msg.points[0].has_time = true;
	strcpy(msg.points[0].type, "temp");
	msg.points[0].value = 23.3;


	uint16_t crc1 = 0;
	int status;
	memset(buffer, 0x00, sizeof(buffer));

	pb_ostream_t stream = pb_ostream_from_buffer(&(buffer[1]), sizeof(buffer) - 3);

	// status = pb_encode(&stream, Point_fields, &temperature_pt[1]);
	status = pb_encode(&stream, siot_Serial_fields, &msg);

	printf("status: %b\n", status);
	printf("bytes_written: %i\n", stream.bytes_written);

	print_hex(buffer, stream.bytes_written);
}

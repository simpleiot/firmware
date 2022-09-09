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

char welcome[] = "SIOT Sender Starting...";
char subj[] = "t";
char temp[] = "temp";
char text[] = "F";
char key_out[] = "out";
char key_in[] = "in";
char blank[] = "";
char zero[] = "0";
char one[] = "1";
char two[] = "2";
char three[] = "3";
char four[] = "4";
char description[] = "description";
char temp_sensor[] = "temp sensor";

float inlet_temperature;
float outlet_temperature;

typedef struct
{
    Point *points;
    uint16_t num_of_points;

} pointsArg_t;

Point temperature_pt[2];
pointsArg_t temperature_points;
siot_Serial message2 = siot_Serial_init_default;

// a callback for providing string variable types
static bool write_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
// static bool write_string(pb_ostream_t *stream, const pb_field_t *field, pb_byte_t *const *arg)
{
    return pb_encode_tag_for_field(stream, field) &&
           pb_encode_string(stream, *arg, strlen(*arg));
}

bool pointsEncode(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    pointsArg_t *pointsArg = (pointsArg_t *)(*arg);
    uint16_t point_index = 0;

    while (point_index < pointsArg->num_of_points) // todo - protect from buffer over-run
    {

        if (!pb_encode_tag_for_field(stream, field))
        {
            return false;
        }

        if (!pb_encode_submessage(stream, Point_fields, (void const *)(&(pointsArg->points[point_index]))))
        {
            return (false);
        }

        point_index++;
    }

    return true;
}

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

bool sendSIOTMsg(siot_Serial *message)
{
    uint8_t buffer[256];
    char print_string[250];
    bool status;
    int i;
    static uint8_t pb_msg_counter = 0;
    uint16_t crc1 = 0;
    memset(buffer, 0x00, 256);

    pb_ostream_t stream = pb_ostream_from_buffer(&(buffer[1]), sizeof(buffer) - 3);

    // status = pb_encode(&stream, Point_fields, &temperature_pt[1]);
    status = pb_encode(&stream, siot_Serial_fields, message);

    buffer[0] = pb_msg_counter;

    crc1 = CRC16K(buffer, (stream.bytes_written + 1));

    buffer[stream.bytes_written + 1] = (uint8_t)((crc1 & 0x00FF));      // CRC Placeholder
    buffer[stream.bytes_written + 2] = (uint8_t)((crc1 & 0xFF00) >> 8); // CRC Placeholder

    pb_msg_counter++;

    if (status)
    {
        cobsWrapper.send(buffer, stream.bytes_written + 3);
        cobsWrapper.update();

        return 0;
    }
    else
    {
        sprintf(print_string, "Encoding failed");
        cobs_print(print_string);
        return 1; /* Failure */
    }
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
    uint8_t buffer[256];
    char print_string[256];
    bool status;
    int i;

    cobsWrapper.setStream(&Serial);

    sprintf(print_string, "Starting COBS wrapped PB test.\r\n");
    cobs_print(print_string);

    inlet_temperature = 0.0;
    outlet_temperature = 12.0;

    siot_Serial message = siot_Serial_init_default;

    temperature_pt[0] = Point_init_default;
    temperature_pt[1] = Point_init_default;

    temperature_pt[0].type.funcs.encode = &write_string;
    temperature_pt[0].type.arg = temp;

    temperature_pt[0].value = 23;

    temperature_pt[0].has_time = true;

    temperature_pt[0].time.seconds = 0;
    temperature_pt[0].time.nanos = 0;

    // temperature_pt[0].text.funcs.encode = &write_string;
    // temperature_pt[0].text.arg = zero;

    temperature_pt[0].key.funcs.encode = &write_string;
    temperature_pt[0].key.arg = one;

    temperature_pt[0].index = 1;

    // temperature_pt[0].data.funcs.encode = &write_string;
    // temperature_pt[0].data.arg = two;

    temperature_pt[1].type.funcs.encode = &write_string;
    temperature_pt[1].type.arg = description;

    // temperature_pt[1].value = 46;

    temperature_pt[1].has_time = true;

    temperature_pt[1].time.seconds = 0;
    temperature_pt[1].time.nanos = 0;

    temperature_pt[1].text.funcs.encode = &write_string;
    temperature_pt[1].text.arg = temp_sensor;

    temperature_pt[1].key.funcs.encode = &write_string;
    temperature_pt[1].key.arg = three;

    temperature_pt[1].index = 2;

    // temperature_pt[1].data.funcs.encode = &write_string;
    // temperature_pt[1].data.arg = four;

    temperature_points.points = temperature_pt;
    temperature_points.num_of_points = 2;

    // bind callback
    message.subject.funcs.encode = &write_string;
    message.subject.arg = welcome;
    // message.points.funcs.encode = &pointsEncode;
    // message.points.arg = &temperature_points;

    // message2.subject.funcs.encode = &write_string;
    // message2.subject.arg = subj;
    message2.points.funcs.encode = &pointsEncode;
    message2.points.arg = &temperature_points;

#if 0
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    // load something into the message via a callback:
    // write_string(&stream, ????, &welcome);
    // punt for now and just assign defaults to the arg field in the message struct

    status = pb_encode(&stream, siot_Serial_fields, &message);

    if (status)
    {
        char str2[10];
        sprintf(print_string, "Number of bytes in stream: %d", stream.bytes_written);
        cobs_print(print_string);
        sprintf(print_string, "PB Bytes: ");
        for (i = 0; i <= stream.bytes_written;)
        {
            sprintf(str2, "%02X ", buffer[i++]);
            strncat(print_string, str2, 10);
        }

        cobs_print(print_string);
        cobsWrapper.send(buffer, stream.bytes_written + 3);
        cobsWrapper.update();
    }
    else
    {
        sprintf(print_string, "Encoding failed");
        cobs_print(print_string);
        return 1; /* Failure */
    }

    // try to decode the buffer:
    pb_istream_t istream = pb_istream_from_buffer(buffer, stream.bytes_written);

    status = pb_decode(&istream, siot_Serial_fields, &message);

#endif
    // Serial.print("Status of decoding: ");

    // Serial.println(status);
}
//
void loop()
{
    uint8_t buffer[256];
    char print_string[250];
    bool status;
    int i;
    static uint16_t msg_counter = 0;
    static uint8_t pb_msg_counter = 0;
    uint16_t crc1 = 0;

    delay(100);

    inlet_temperature = inlet_temperature + 5.0;

    outlet_temperature = inlet_temperature + 5.0;

    sprintf(print_string, "************* Mini-ATS COBS Test Msg: %08d *************", msg_counter, inlet_temperature, outlet_temperature);
    cobs_print(print_string);

    msg_counter++;

    memset(buffer, 0x00, 256);

    temperature_pt[0].value = inlet_temperature;

    sendSIOTMsg(&message2);

#if 0
    // temperature_pt[1].value = 10.0; // outlet_temperature;

    // pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    pb_ostream_t stream = pb_ostream_from_buffer(&(buffer[1]), sizeof(buffer) - 3);

    // status = pb_encode(&stream, Point_fields, &temperature_pt[1]);
    status = pb_encode(&stream, siot_Serial_fields, &message2);

    buffer[0] = pb_msg_counter;

    crc1 = CRC16K(buffer, (stream.bytes_written + 1));

    buffer[stream.bytes_written + 1] = (uint8_t)((crc1 & 0x00FF));      // CRC Placeholder
    buffer[stream.bytes_written + 2] = (uint8_t)((crc1 & 0xFF00) >> 8); // CRC Placeholder

    pb_msg_counter++;

    if (status)
    {
        char str2[10];
        // sprintf(print_string, "Number of bytes in stream: %d", stream.bytes_written + 4);
        sprintf(print_string, "Number of bytes in stream: %d CRC = %04X", stream.bytes_written + 3, crc1);
        cobs_print(print_string);
#if 0
        sprintf(print_string, "PB Bytes: ");
        for (i = 0; i <= stream.bytes_written;)
        {
            sprintf(str2, "%02X ", buffer[i++]);
            strncat(print_string, str2, 10);
        }

        cobs_print(print_string);
#endif
        cobsWrapper.send(buffer, stream.bytes_written + 3);
        // cobsWrapper.send(buffer, stream.bytes_written);
        //  cobsWrapper.send(buffer, stream.bytes_written);
        cobsWrapper.update();
    }
    else
    {
        sprintf(print_string, "Encoding failed");
        cobs_print(print_string);
        return 1; /* Failure */
    }

#endif

#if 0
    memset(buffer, 0x00, 256);
    // try to decode the buffer:
    pb_istream_t istream = pb_istream_from_buffer(buffer, stream.bytes_written);

    status = pb_decode(&istream, siot_Serial_fields, &temperature_pt[0]);

#endif
}

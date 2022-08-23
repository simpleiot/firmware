#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <pb.h>

#include <siot_serial.pb.h>
#include <point.pb.h>

// NOTE: maximum receive buffer length in Uno default serial ISR is 64 bytes.

char welcome[] = "Our first SIOT message!";
char type[] = "temp";
char text[] = "degress F";
char key_out[] = "out";
char key_in[] = "in";

float inlet_temperature;
float outlet_temperature;
Point out_point;
Point in_point;

// a callback for providing string variable types
static bool write_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
// static bool write_string(pb_ostream_t *stream, const pb_field_t *field, pb_byte_t *const *arg)
{
    return pb_encode_tag_for_field(stream, field) &&
           pb_encode_string(stream, *arg, strlen(*arg));
}

// the setup function runs once when you press reset or power the board
void setup()
{
    Serial.begin(115200);
    uint8_t buffer[256];
    bool status;
    int i;

    inlet_temperature = 0.0;
    outlet_temperature = 12.0;

    siot_Serial message = siot_Serial_init_default;

    out_point = Point_init_default;

    out_point.type.funcs.encode = &write_string;
    out_point.type.arg = type;
    out_point.text.funcs.encode = &write_string;
    out_point.text.arg = text;
    out_point.key.funcs.encode = &write_string;
    out_point.key.arg = key_out;

    out_point.value = 0.0;
    out_point.index = 0.0;
    out_point.has_time = false;

    in_point = Point_init_default;

    in_point.value = 99.0;
    in_point.index = 99.0;
    in_point.has_time = true;

    // bind callback
    message.subject.funcs.encode = &write_string;
    message.subject.arg = welcome;

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    // load something into the message via a callback:
    // write_string(&stream, ????, &welcome);
    // punt for now and just assign defaults to the arg field in the message struct

    status = pb_encode(&stream, siot_Serial_fields, &message);

    if (status)
    {
        Serial.print("Number of bytes in stream: ");
        Serial.println(stream.bytes_written);
        for (i = 0; i <= stream.bytes_written;)
        {
            Serial.print(buffer[i++], HEX);
        }

        Serial.println();
    }
    else
    {
        Serial.println("Encoding failed");
        return 1; /* Failure */
    }

    // try to decode the buffer:
    pb_istream_t istream = pb_istream_from_buffer(buffer, stream.bytes_written);

    status = pb_decode(&istream, siot_Serial_fields, &message);

    Serial.print("Status of decoding: ");
    Serial.println(status);
}
//
void loop()
{
    uint8_t buffer[256];
    char print_string[250];
    bool status;
    int i;

    Serial.println(" ");
    Serial.println(" ");
    Serial.println("-----------------------------");
    Serial.println("-----------------------------");

    inlet_temperature = inlet_temperature + 5.0;

    outlet_temperature = inlet_temperature + 5.0;

    Serial.print("inlet_temperature    = ");
    Serial.println(inlet_temperature);
    Serial.print("outlet_temperature   = ");
    Serial.println(outlet_temperature);
    Serial.println("--------------");

    memset(buffer, 0x00, 256);

    out_point.key.arg = key_in;
    out_point.index = 0.0;

    out_point.value = inlet_temperature;
    Serial.println(" ");
    Serial.println(" ");
    Serial.println("-----------------------------");
    Serial.println("-----------------------------");

    Serial.println("--------------");

    Serial.println("Point to encode:");

    Serial.print("  out_point.type      = ");
    sprintf(print_string, "%s", out_point.type.arg);
    Serial.println(print_string);

    Serial.print("  out_point.key       = ");
    sprintf(print_string, "%s", out_point.key.arg);
    Serial.println(print_string);

    Serial.print("  out_point.value     = ");
    Serial.println(out_point.value);

    Serial.print("  out_point.text      = ");
    sprintf(print_string, "%s", out_point.text.arg);
    Serial.println(print_string);

    Serial.print("  out_point.index     = ");
    Serial.println(out_point.index);

    Serial.print("  out_point.has_time  = ");
    Serial.println(out_point.has_time);

    Serial.print("  out_point.tombstone = ");
    Serial.println(out_point.tombstone);

    Serial.println("--------------");

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    status = pb_encode(&stream, Point_fields, &out_point);

    if (status)
    {
        Serial.print("Number of bytes in stream: ");
        Serial.println(stream.bytes_written);
        for (i = 0; i <= stream.bytes_written;)
        {
            Serial.print(buffer[i++], HEX);
        }

        Serial.println();
    }
    else
    {
        Serial.println("Encoding failed");
        return 1; /* Failure */
    }

    memset(buffer, 0x00, 256);
    // try to decode the buffer:
    pb_istream_t istream = pb_istream_from_buffer(buffer, stream.bytes_written);

    status = pb_decode(&istream, Point_fields, &in_point);

    Serial.print("Status of decoding: ");
    Serial.println(status);

// ToDo: Point Decoding
#if 0    

    Serial.println("--------------");

    Serial.println("Decoded point:");

    Serial.print("  in_point.type      = ");
    sprintf(print_string, "%s", in_point.type.arg);
    Serial.println(print_string);

    Serial.print("  in_point.key       = ");
    sprintf(print_string, "%s", in_point.key.arg);
    Serial.println(print_string);

    Serial.print("  in_point.value     = ");
    Serial.println(in_point.value);

    Serial.print("  in_point.text      = ");
    sprintf(print_string, "%s", in_point.text.arg);
    Serial.println(print_string);

    Serial.print("  in_point.index     = ");
    Serial.println(in_point.index);

    Serial.print("  in_point.has_time  = ");
    Serial.println(in_point.has_time);

    Serial.print("  in_point.tombstone = ");
    Serial.println(in_point.tombstone);

    Serial.println("--------------");
#endif

    memset(buffer, 0x00, 256);

    out_point.key.arg = key_out;
    out_point.index = 1.0;

    out_point.value = outlet_temperature;
    Serial.println(" ");
    Serial.println(" ");
    Serial.println("-----------------------------");
    Serial.println("-----------------------------");

    Serial.println("--------------");

    Serial.println("Point to encode:");

    Serial.print("  out_point.type      = ");
    sprintf(print_string, "%s", out_point.type.arg);
    Serial.println(print_string);

    Serial.print("  out_point.key       = ");
    sprintf(print_string, "%s", out_point.key.arg);
    Serial.println(print_string);

    Serial.print("  out_point.value     = ");
    Serial.println(out_point.value);

    Serial.print("  out_point.text      = ");
    sprintf(print_string, "%s", out_point.text.arg);
    Serial.println(print_string);

    Serial.print("  out_point.index     = ");
    Serial.println(out_point.index);

    Serial.print("  out_point.has_time  = ");
    Serial.println(out_point.has_time);

    Serial.print("  out_point.tombstone = ");
    Serial.println(out_point.tombstone);

    Serial.println("--------------");

    stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    status = pb_encode(&stream, Point_fields, &out_point);

    if (status)
    {
        Serial.print("Number of bytes in stream: ");
        Serial.println(stream.bytes_written);
        for (i = 0; i <= stream.bytes_written;)
        {
            Serial.print(buffer[i++], HEX);
        }

        Serial.println();
    }
    else
    {
        Serial.println("Encoding failed");
        return 1; /* Failure */
    }

    memset(buffer, 0x00, 256);

    // try to decode the buffer:
    istream = pb_istream_from_buffer(buffer, stream.bytes_written);

    status = pb_decode(&istream, Point_fields, &in_point);

    Serial.print("Status of decoding: ");
    Serial.println(status);

// ToDo: Point Decoding
#if 0    

    Serial.println("--------------");

    Serial.println("Decoded point:");

    Serial.print("  in_point.type      = ");
    sprintf(print_string, "%s", in_point.type.arg);
    Serial.println(print_string);

    Serial.print("  in_point.key       = ");
    sprintf(print_string, "%s", in_point.key.arg);
    Serial.println(print_string);

    Serial.print("  in_point.value     = ");
    Serial.println(in_point.value);

    Serial.print("  in_point.text      = ");
    sprintf(print_string, "%s", in_point.text.arg);
    Serial.println(print_string);

    Serial.print("  in_point.index     = ");
    Serial.println(in_point.index);

    Serial.print("  in_point.has_time  = ");
    Serial.println(in_point.has_time);

    Serial.print("  in_point.tombstone = ");
    Serial.println(in_point.tombstone);

    Serial.println("--------------");

#endif

    delay(10000);
}

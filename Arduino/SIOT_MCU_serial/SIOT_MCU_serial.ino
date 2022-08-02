#include <nanopb/pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <pb.h>

#include <pb/siot_serial.pb.h>

//NOTE: maximum receive buffer length in Uno default serial ISR is 64 bytes.

char welcome[] = "Our first SIOT message!";

// a callback for providing string variable types
static bool write_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
    return pb_encode_tag_for_field(stream, field) &&
           pb_encode_string(stream, *arg, strlen(*arg));
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT); //leftover from test code
  Serial.begin(115200);
  uint8_t buffer[256];
  bool status;
  int i;

  siot_Serial message = siot_Serial_init_default;

  // bind callback
  message.subject.funcs.encode = &write_string;
  message.subject.arg = welcome;

  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  //load something into the message via a callback:
  //write_string(&stream, ????, &welcome);
  //punt for now and just assign defaults to the arg field in the message struct

  status = pb_encode(&stream, siot_Serial_fields, &message);

     if (status)
        {
            Serial.print("Number of bytes in stream: ");
            Serial.println(stream.bytes_written);
            for (i=0; i<=stream.bytes_written;)
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
void loop() {
 ;
}

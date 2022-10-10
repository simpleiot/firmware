#include <PacketSerial.h>

PacketSerial cobsWrapper;


uint16_t msg_counter;

void onSerialReceived(const uint8_t* buffer, size_t size) {
  // TODO
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115000);
  cobsWrapper.setStream(&Serial);
  msg_counter = 0;
}

void loop() {
  char cobsTest2Array[40];

  memset(cobsTest2Array, 0x00, 40);

  sprintf( cobsTest2Array, "Mini-ATS COBS Test Msg: %08d", msg_counter);

  msg_counter++;


  cobsWrapper.send(cobsTest2Array, 35);
  cobsWrapper.update();
  delay(1000);
}

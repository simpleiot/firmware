#include <PacketSerial.h>

PacketSerial cobsWrapper;

void onSerialReceived(const uint8_t* buffer, size_t size) {
  // TODO
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115000);
  cobsWrapper.setStream(&Serial);
}

void loop() {
  static const char msg[] = "Hi Cliff";
  cobsWrapper.send(msg, sizeof(msg));
  cobsWrapper.update();
  delay(1000);
}

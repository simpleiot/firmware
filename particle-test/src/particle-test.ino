/*
 * Project particle-test
 * Description:
 * Author:
 * Date:
 */

#include "dct.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(MANUAL);

// setup() runs once, when the device is first turned on.
void setup()
{
  const uint8_t val = 0x01;
  dct_write_app_data(&val, DCT_SETUP_DONE_OFFSET, 1);
  Serial.begin(115200);
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  // The core of your code will likely live here.
  Serial.println("loop");
  delay(1000);
}
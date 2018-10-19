#include <Adafruit_BME280.h>
#include <ArduinoJson.h>

int led = D6;

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

// Having declared these variables, let's move on to the setup function.
// The setup function is a standard part of any microcontroller program.
// It runs only once when the device boots up or is reset.

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(115200);

  bool status;

  status = bme.begin(0x76);

  if (!status) {
    Serial.println("Could not find BME280 sensor, check wiring");
  }


}

// Next we have the loop function, the other essential part of a microcontroller program.
// This routine gets repeated over and over, as quickly as possible and as many times as possible, after the setup function is called.
// Note: Code that blocks for too long (like more than 5 seconds), can make weird things happen (like dropping the network connection).  The built-in delay function shown below safely interleaves required background activity, so arbitrarily long delays can safely be done if you need them.

int lastReport = 0;
float lastTemp = -100000;
float lastHum = -100000;

// fill in the following with the IP address of your influxdb server
IPAddress ip = IPAddress(1,2,3,4);

UDP Udp;

void send_influx_data(float temp, float humidity, float pressure) {
	String data = "sensor T=" + String(temp, 1) +
		",H=" + String(humidity, 1) +
		",P=" + String(pressure, 1);
	Udp.begin(8089);
	Udp.beginPacket(ip, 8089);
	Serial.printlnf("Publishing: %s", data.c_str());
	Udp.write(data);
	int sent = Udp.endPacket();
	Serial.printlnf("%i bytes sent", sent);
	Udp.stop();
}

void loop() {
  // To blink the LED, first we'll turn it on...
  digitalWrite(led, HIGH);

  // We'll leave it on for 1 second...
  delay(1000);

  // Then we'll turn it off...
  digitalWrite(led, LOW);

  // Wait 1 second...
  delay(1000);

  float temp = bme.readTemperature() * 1.8 + 32;
  float pres = bme.readPressure();
  float hum = bme.readHumidity();

  Serial.printlnf("Temperature: %f C", temp);
  Serial.printlnf("Pressure: %f hPa", pres);
  Serial.printlnf("Humidity: %f %%", hum);

  int now = Time.now();
  bool publish = false;

  if (now - lastReport > 60*15) {
	  Serial.println("15 minute publish");
	  publish = true;
  }

  if (abs(temp - lastTemp) > 0.5 && now - lastReport > 60) {
	Serial.println("temp needs reported");
	publish = true;
  }

  if (abs(hum - lastHum) > 0.5 && now - lastReport > 60) {
	Serial.println("humidity needs reported");
	publish = true;
  }

  if (publish) {
	send_influx_data(temp, hum, pres);
	lastReport = now;
	lastTemp = temp;
	lastHum = hum;
  }

  // And repeat!
}

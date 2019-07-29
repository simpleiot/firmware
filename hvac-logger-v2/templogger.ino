
void setup() {
  Serial.begin(115200);
  Serial.println("HVAC Logger v2");
}


float tempSim = 60;

void loop() {
  // We'll leave it on for 1 second...
  delay(1000*60);
  tempSim += 1;
  if (tempSim > 80) {
	  tempSim = 60;
  }
  String data = String::format("{\"values\":{\"temp\":%f}}", tempSim);
  Serial.println(data);
  Particle.publish("siot", data, PRIVATE);
}

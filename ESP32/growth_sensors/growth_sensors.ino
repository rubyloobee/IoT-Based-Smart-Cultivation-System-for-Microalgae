#include <Wire.h>

void setup() {
  Serial.begin(115200);
  
  setupColourSensor();
  
  Serial.println(F("System Ready. Monitoring Algae Culture..."));
}

void loop() {
  Serial.println(F("\n==============================="));
  
  // Obtain colour sensor reading
  setSensorLED(true);
  readColourSensor();
  setSensorLED(false);
  Serial.println(F("-------------------------------"));

  // Obtain turbidity sensor reading
  readTurbiditySensor();
  
  Serial.println(F("==============================="));

  delay(5000); 
}
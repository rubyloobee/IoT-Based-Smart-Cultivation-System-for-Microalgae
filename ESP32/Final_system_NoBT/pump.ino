#include "actuator.h"

void setupPump() {
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);

   // Start with Pump OFF (Active Low)
  digitalWrite(RELAY_1, HIGH);
  digitalWrite(RELAY_2, HIGH);
  digitalWrite(RELAY_3, HIGH);
  digitalWrite(RELAY_4, HIGH);
}

void samplingPump () {
  digitalWrite(RELAY_1, LOW);
  delay(300);
  digitalWrite(RELAY_1, HIGH);
  delay(100);
}

void adjustWaterLevel() {
  // If sensor fails / not connected properly
  if (waterLevel < -1) { // 
    digitalWrite(RELAY_2, HIGH);
    return;
  }

  int buffer = 5; 
  float gap = (float)target_water_level - waterLevel;

  // If gap is large, turn on pump
  if (gap > buffer) {
    digitalWrite(RELAY_2, LOW); // ON
    Serial.printf("[Pump] ON | Level: %.1f%% | Target: %d%%\n", waterLevel, target_water_level);
  } 
  // If gap is smaller than 5%, turn on pump for 2s
  else if (gap > 0){
    digitalWrite(RELAY_2, LOW);
    delay(150); 
    digitalWrite(RELAY_2, HIGH);
  }
  // Water level is reached
  else {
    digitalWrite(RELAY_2, HIGH); // OFF
  }
}

void nutrientPump() {
  digitalWrite(RELAY_3, LOW);
  delay(300);
  digitalWrite(RELAY_3, HIGH);
  delay(100);
}

void harvestPump() {
  digitalWrite(RELAY_4, LOW);
  delay(300);
  digitalWrite(RELAY_4, HIGH);
  delay(100);
}


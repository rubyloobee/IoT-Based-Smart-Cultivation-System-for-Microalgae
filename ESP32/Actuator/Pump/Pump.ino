// CHANGE BELOW ACCORDING TO PIN CONNECTED TO ESP32
#define RELAY1 32
#define RELAY2 33
#define RELAY3 25
#define RELAY4 26

void setup() {
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);

  // Set OFF level first to avoid glitches
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  digitalWrite(RELAY3, HIGH);
  digitalWrite(RELAY4, LOW);
  delay(3000);
  digitalWrite(RELAY4, HIGH);
}

void loop() {
  digitalWrite(RELAY1, LOW);  // Pump 1 ON
  delay(3000);

  digitalWrite(RELAY1, HIGH); // Pump 1 OFF
  digitalWrite(RELAY2, LOW);  // Pump 2 ON
  delay(3000);

  digitalWrite(RELAY2, HIGH); // Pump 2 OFF
  digitalWrite(RELAY3, LOW);  // Pump 3 ON
  delay(3000);

  digitalWrite(RELAY3, HIGH); // Pump 3 OFF
  digitalWrite(RELAY4, LOW);  // Pump 4 ON
  delay(3000);

  digitalWrite(RELAY4, HIGH); // Pump 4 OFF
  delay(3000);
}

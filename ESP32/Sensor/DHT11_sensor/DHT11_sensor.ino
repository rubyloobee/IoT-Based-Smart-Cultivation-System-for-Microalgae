#include "DHT.h"

#define DHTPIN 4      // CHANGE ACCORDING TO PIN CONNECTED TO ESP32
#define DHTTYPE DHT11 

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println(F("DHT11 Temperature Test"));

  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements (DHT11 is slow)
  delay(2000);

  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Temperature: "));
  Serial.print(t);
  Serial.println(F("°C"));
}
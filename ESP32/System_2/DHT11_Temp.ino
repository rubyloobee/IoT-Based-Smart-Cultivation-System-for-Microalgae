#include "sensor.h"

// Setup a DHT instance
DHT dht(DHTPIN, DHTTYPE);

// Configuration
const unsigned long TEMP_READ_DELAY = 2000; // DHT11 needs 2 seconds between reads

// --- Variables ---
float tempReadings[NUM_SAMPLES];
float tempAvg = 0.0;

// Setup function for the DHT11 sensor
void setupTempSensor() {
  // Start the DHT library
  dht.begin();
  Serial.println("DHT11 Sensor Initialized");
}

// Dedicated loop logic function for the DHT11 sensor
float readTempSensor() {
  Serial.println(F("\n--- Reading DHT11 (Temperature) ---"));

  // Ensure there is 2 seconds delay between readings
  float temp = dht.readTemperature();

  // Sensor fails (returns NaN if it can't communicate)
  if (isnan(temp)) {
    Serial.println("Error: DHT11 not detected or read failed!");
    return ERROR_FLAG;
  } 

  // Print the single result
  Serial.printf("Current Temperature: %.2f °C\n", temp);

  return temp;
}
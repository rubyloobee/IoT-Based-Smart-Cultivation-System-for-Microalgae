#include <OneWire.h>           // Handle OneWire communication protocol
#include <DallasTemperature.h> // Built on top of OneWire to make working with temperature sensors easy

// CHANGE ACCORDING TO PIN CONNECTED TO ESP32
#define ONE_WIRE_BUS 27

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

// Configuration
const int NUM_SAMPLES = 10;     // number of readings to average
const unsigned long tempSampleTime = 20000U; 
const unsigned long conversionDelay = 750U; 

// Globals
float readings[NUM_SAMPLES];
float tempAvg = 0.0;


// Computes moving average of stored readings, ignoring error flags
float computeAverage() {
  float total = 0;
  float validSamples = 0;

  // Define the error flag value for comparison
  const float ERROR_FLAG = -999.0; 

  for (int i = 0; i < NUM_SAMPLES; i++) {
    // Check if the reading is NOT the error flag
    if (readings[i] != ERROR_FLAG) {
      total += readings[i];
      validSamples++; // Only count valid samples
    }
  }
  
  // Prevent division by zero if all samples were errors
  if (validSamples == 0) {
    return -999.0; // Indicate that no valid data was found in this cycle
  }
  
  return (float) total / validSamples;
}

void setup() {
  Serial.begin(9600);
  sensors.begin();  
}

void loop() {
  // initialise tempLastReading with a large value
  // (tempLastReading wraps negative number to a massive number)
  // Runs if statement at the power up of esp32 for initial sensor reading
  static unsigned long tempLastReading = 0 - tempSampleTime;


  if (millis() - tempLastReading >= tempSampleTime) {
    // Reset timer
    tempLastReading = millis();

    for (int i = 0; i < NUM_SAMPLES; i++){ 
      sensors.requestTemperatures();

      delay(conversionDelay);

      // Correction equation after calibration
      float temp = sensors.getTempCByIndex(0);
      // CHANGE CALIBRATION EQUATION
      float correctedTemp = 0.9988 * temp + 0.0807;

      // Sensor fails
      if (temp == DEVICE_DISCONNECTED_C) {
        Serial.println("Error: DS18B20 not detected!");
        //Store the error flag
        readings[i] = -999.0; 
      } else {
        // Valid reading
        readings[i] = correctedTemp;
      }   
    }
    // Computer average
    tempAvg = computeAverage();

    Serial.print("10 Temp Readings: ");
    for (int i = 0; i < NUM_SAMPLES; i++) {
        Serial.print(readings[i]);
        if (i < NUM_SAMPLES - 1) {
            Serial.print(", ");
        }
    }
    Serial.println(); 

    Serial.print("Temp Average: ");
    Serial.print(tempAvg);
    Serial.println(" °C");
    Serial.println("-------------------------------------");
  }
}



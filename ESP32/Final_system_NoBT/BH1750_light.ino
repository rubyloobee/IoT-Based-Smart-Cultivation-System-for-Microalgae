#include <BH1750.h>            
#include <Wire.h>              
#include "sensor.h"

BH1750 lightMeter;

const unsigned long LIGHT_CONVERSION_DELAY = 180U; // Minimum delay for HIGH_RES_MODE is ~120ms

float luxReadings[NUM_SAMPLES];
float luxAvg = 0.0;

// Setup function for the BH1750 sensor
void setupLightSensor() {
  // Initialize the BH1750 sensor in Continuous High-Res Mode
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire1)) {
    Serial.println("BH1750 not found!");
  }
}

float readLightSensor(bool print) {
  for (int i = 0; i < NUM_SAMPLES; i++){ 
        float lux = lightMeter.readLightLevel();

    if (lux == 0.0) {
      Serial.println("Error: BH1750 returned 0.0 (Possible comm failure)");
      luxReadings[i] = -999.0; // Store the error flag
    } 
    // Valid reading
    else { 
      // CHANGE CALIBRAITON EQUATION
      float correctedLux = 0.996 * lux - 0.2625;
      
      luxReadings[i] = correctedLux;
    } 
      delay(LIGHT_CONVERSION_DELAY);
  }

  luxAvg = computeAverage(luxReadings, NUM_SAMPLES);
  if (print){
    Serial.println(F("\n--- Reading BH1750 (Light) ---"));
    printResults("Lux", luxReadings, luxAvg, "lux", NUM_SAMPLES);
  }
  
  return luxAvg;
}
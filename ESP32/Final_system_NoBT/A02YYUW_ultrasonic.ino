#include "sensor.h"

// CHANGE TO YOUR TANK HEIGH IN CM
float TANK_HEIGHT = 24.0;    

float waterLevelReadings[NUM_SAMPLES];
float waterLevelAvg = 0.0;
static int bufferIndex = 0;     

// Setup function for the A02YYUW sensor
void setupUltrasonicSensor() {
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
}


// Reads 4-byte frame from A02YYUW and validates checksum
// Returns distance in mm
bool readSensor(int &distance) {
  // The sensor sends data continuously, wait for a full frame (4 bytes)
  if (Serial2.available() >= 4) {
    uint8_t header = Serial2.read();
    // Start reading at the correct header (0xFF)
    if (header == 0xFF) {
      uint8_t high = Serial2.read();
      uint8_t low = Serial2.read();
      uint8_t sum = Serial2.read();
      uint8_t checksum = (0xFF + high + low) & 0xFF;

      // Data integrity checking
      if (sum == checksum) {
        distance = (high << 8) + low;
        return true; // valid reading
      }
    }
  }
  // Transmitted checksum != computed checksum -> invalid / incomplete frame
  return false; 
}

// Polls the sensor continuously to keep the buffer fresh
void pollSensorFast() {
  int distance_mm = 0;
  
  // If readSensor returns a new valid frame, store it globally
  if (readSensor(distance_mm)) {
    // Convert distance from mm to cm
    float distance_cm = distance_mm / 10.0;
    // CHANGE CALIBRAITON EQUATION
    float correctedDistance = 0.9573 * distance_cm - 0.2435;
    // Water Level = Tank Height - Measured Distance from sensor
    float correctedWaterLevel = TANK_HEIGHT - correctedDistance;

    waterLevelReadings[bufferIndex] = correctedWaterLevel;
    // Update index for next read
    bufferIndex = (bufferIndex + 1) % NUM_SAMPLES;
  }  
}

// Loop logic function for the A02YYUW sensor
float readUltrasonicSensor(bool print) {
  
  // Get water level in percentage
  float waterLevelAvgCm = computeAverage(waterLevelReadings, NUM_SAMPLES);

  // Water level is at impossible value / far above tank height
  if (waterLevelAvgCm < -5.0) {
    return ERROR_FLAG;
  }

  float currentLevelPct = (waterLevelAvgCm / TANK_HEIGHT) * 100.0;
  currentLevelPct = constrain(currentLevelPct, 0.0, 100.0);
  if (print){
    Serial.println(F("\n--- Reading A02YYUW (Water Level) ---"));
    printResults("Water Level", waterLevelReadings, currentLevelPct, "%", NUM_SAMPLES);
  }

  return currentLevelPct;
}



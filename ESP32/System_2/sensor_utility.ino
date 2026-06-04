#include "sensor.h"

// Computes moving average of stored readings for any sensor type, ignoring error flags
float computeAverage(float* readings, int size) {
  float total = 0;
  float validSamples = 0;

  for (int i = 0; i < size; i++) {
    // Check if the reading is NOT the error flag
    if (readings[i] != ERROR_FLAG) {
      total += readings[i];
      validSamples++; // Only count valid samples
    }
  }
  
  // Prevent division by zero if all samples were errors
  if (validSamples == 0) {
    return ERROR_FLAG; // Indicate that no valid data was found in this cycle
  }
  
  return (float) total / validSamples;
}

// Prints the individual readings and the final average for any sensor
void printResults(const char* sensorName, float* readings, float avg, const char* unit, int size) {
  Serial.print(size);
  Serial.print(" ");
  Serial.print(sensorName);
  Serial.print(" Readings: ");

  // Print individual readings
  for (int i = 0; i < size; i++) {
      // Check if it's an error flag before printing
      if (readings[i] == ERROR_FLAG) {
        Serial.print("ERR");
      } else {
        Serial.print(readings[i]);
      }
      
      if (i < size - 1) {
        Serial.print(", ");
      }
  }
  Serial.println(); 

  // Print final average
  Serial.print(sensorName);
  Serial.print(" Average: ");
  if (avg == ERROR_FLAG) {
  Serial.println("ERROR - No valid data");
  } else {
  Serial.print(avg); 
  Serial.print(" ");
  Serial.println(unit);
  }
  Serial.println("-------------------------------------");
}
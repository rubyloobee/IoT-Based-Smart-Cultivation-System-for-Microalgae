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
    return ERROR_FLAG;
  }
  
  return (float) total / validSamples;
}

// Prints the individual readings and the final average for any sensor
void printResults(const char* sensorName, float* readings, float avg, const char* unit, int size) {
  SerialBT.print(size);
  SerialBT.print(" ");
  SerialBT.print(sensorName);
  SerialBT.print(" Readings: ");

  // Print individual readings
  for (int i = 0; i < size; i++) {
      if (readings[i] == ERROR_FLAG) {
        SerialBT.print("ERR");
      } else {
        SerialBT.print(readings[i]);
      }
      
      if (i < size - 1) {
        SerialBT.print(", ");
      }
  }
  SerialBT.println(); 

  // Print final average
  SerialBT.print(sensorName);
  SerialBT.print(" Average: ");
  if (avg == ERROR_FLAG) {
  SerialBT.println("ERROR - No valid data");
  } else {
  SerialBT.print(avg); 
  SerialBT.print(" ");
  SerialBT.println(unit);
  }
  SerialBT.println("-------------------------------------");
}
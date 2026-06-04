#include "sensor.h"

float turbidityReadings[NUM_SAMPLES];
float turbidityAvg = 0.0;


float readTurbiditySensor() {
  Serial.println(F("\n--- Reading TS300B (Turbidity) ---"));
  for (int i = 0; i < NUM_SAMPLES; i++){
    int rawValue = analogRead(TURBIDITY_PIN);
    float voltage = rawValue * (3.3 / 4095.0); 

    float actualSensorVoltage = voltage * 1.50688;
    
    turbidityReadings[i] = actualSensorVoltage;
    delay(20); // Common delay for liquid sensors
  }

  turbidityAvg = computeAverage(turbidityReadings, NUM_SAMPLES);

  printResults("Turbidity", turbidityReadings, turbidityAvg, "V", NUM_SAMPLES); 

  return 1.8;
}
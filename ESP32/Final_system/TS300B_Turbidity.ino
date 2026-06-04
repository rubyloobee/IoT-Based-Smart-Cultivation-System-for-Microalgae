#include "sensor.h"

float turbidityReadings[NUM_SAMPLES];
float turbidityAvg = 0.0;


float readTurbiditySensor() {
  SerialBT.println(F("\n--- Reading TS300B (Turbidity) ---"));
  for (int i = 0; i < NUM_SAMPLES; i++){
    int rawValue = analogRead(TURBIDITY_PIN);
    float voltage = rawValue * (3.3 / 4095.0); 

    float actualSensorVoltage = voltage * 1.50688;
    // CHANGE CALIBRAITON EQUATION
    float Absorbance = -0.7357 * actualSensorVoltage + 4.2554;
    
    turbidityReadings[i] = Absorbance;
    delay(20); // Common delay for liquid sensors
  }

  turbidityAvg = computeAverage(turbidityReadings, NUM_SAMPLES);

  printResults("Absorbance", turbidityReadings, turbidityAvg, "abs", NUM_SAMPLES); 

  return turbidityAvg;
}
float turbidityReadings[NUM_SAMPLES];
float turbidityAvg = 0.0;

float computeAverage(float* readings, int size) {
  float total = 0;
  for (int i = 0; i < size; i++) {
    total += readings[i];
   }
  return (float) total / size;
}

void printResults(const char* sensorName, float* readings, float avg, const char* unit, int size) {
  Serial.print(size);
  Serial.print(" ");
  Serial.print(sensorName);
  Serial.print(" Readings: ");

  // Print individual readings
  for (int i = 0; i < size; i++) {
    // Check if it's an error flag before printing
    Serial.print(readings[i]);
    
    if (i < size - 1) {
      Serial.print(", ");
    }
  }
  Serial.println(); 

  // Print final average
  Serial.print(sensorName);
  Serial.print(" Average: ");

  Serial.print(avg); 
  Serial.print(" ");
  Serial.println(unit);

  Serial.println("-------------------------------------");
}

void readTurbiditySensor() {
  Serial.println(F("\n--- Reading TS300B (Turbidity) ---"));
  for (int i = 0; i < NUM_SAMPLES; i++){
    int rawValue = analogRead(TURBIDITY_PIN);
    float voltage = rawValue * (3.3 / 4095.0); 

    float actualSensorVoltage = voltage * 1.50688;
    // CHANGE CALIBRAITON EQUATION
    float Absorbance = -0.7357 * actualSensorVoltage + 4.2554;
    
    turbidityReadings[i] = Absorbance;
    delay(20); // Common delay for liquid sensors
  }

  // Compute average
  turbidityAvg = computeAverage(turbidityReadings, NUM_SAMPLES);

  // Print results
  printResults("Absorbance", turbidityReadings, turbidityAvg, "V", NUM_SAMPLES); 

  delay(2000); // Wait 2 seconds between "Growth Checks"
}
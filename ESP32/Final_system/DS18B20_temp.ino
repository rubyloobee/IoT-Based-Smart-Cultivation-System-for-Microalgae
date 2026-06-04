#include <OneWire.h>           
#include <DallasTemperature.h> 
#include "sensor.h"


// Setup a oneWire instance
// 1. Instance for main tank
OneWire oneWireMain(ONE_WIRE_BUS_MAIN);
DallasTemperature sensorsMain(&oneWireMain);

// 2. Instance for sampling tank
OneWire oneWireSample(ONE_WIRE_BUS_SAMPLE);
DallasTemperature sensorsSample(&oneWireSample);

const unsigned long TEMP_CONVERSION_DELAY = 188; // Minimum conversion time for 10-bit resolution

float tempReadings[NUM_SAMPLES];
float tempAvg = 0.0;

// Setup function for the DS18B20 sensor
void setupTempSensor() {
  // Start the Dallas Temperature library
  sensorsMain.begin();        
  sensorsSample.begin();
  sensorsMain.setWaitForConversion(false);
  sensorsMain.setResolution(10); // 10-bit : precision 0.25 celsius
  sensorsSample.setWaitForConversion(false);
  sensorsSample.setResolution(10); // 10-bit : precision 0.25 celsius
}

// Dedicated loop logic function for the DS18B20 sensor
float readTempSensor(int pin) {
  // Determine which temperature sensor is read
  DallasTemperature* tempSensor = NULL;
  String sensorName = "";

  if (pin == ONE_WIRE_BUS_MAIN) {
    tempSensor = &sensorsMain;
    sensorName = "Main Tank";
  } else if (pin == ONE_WIRE_BUS_SAMPLE) {
    tempSensor = &sensorsSample;
    sensorName = "Sample Tank";
  }

  SerialBT.printf("\n--- Reading %s DS18B20 (Temperature) ---\n", sensorName.c_str());

  for (int i = 0; i < NUM_SAMPLES; i++){ 
    // 1. Request measurement
    tempSensor->requestTemperatures();

    // 2. Wait for the 750ms conversion time to complete
    delay(TEMP_CONVERSION_DELAY); 

    // 3. Read the new temperature
    float temp = tempSensor->getTempCByIndex(0);
    

    // Sensor fails
    if (temp == DEVICE_DISCONNECTED_C) {
      SerialBT.println("Error: DS18B20 not detected!");
      tempReadings[i] = -999.0; // Store the error flag
    } 
    // Valid reading
    else {
      // CHANGE CALIBRATION EQUATION
      float correctedTemp = 0.9988 * temp + 0.0807;

      tempReadings[i] = correctedTemp;
    }
  }
  tempAvg = computeAverage(tempReadings, NUM_SAMPLES);
  printResults("Temp", tempReadings, tempAvg, "°C", NUM_SAMPLES);
  return tempAvg;
}
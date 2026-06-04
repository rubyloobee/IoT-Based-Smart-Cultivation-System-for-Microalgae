#include "DFRobot_PH.h"        // Primary library for pH calculation and calibration
#include "EEPROM.h"            // Used by the pH library for calibration storage
#include "sensor.h"

#define PH_CHANNEL                0   // ADS1115 analog channel A0
const byte PH_CALIBRATION_ADDRESS = 0x20;   // EEPROM address for pH calibration data

DFRobot_PH phSensor;

float pHReadings[NUM_SAMPLES];
float pHAvg = 0.0;
float pHvoltage = 0.0;
float pHValue = 0.0;

void setuppHSensor() {
  phSensor.begin();
}

float readpHSensor(float temp){
  SerialBT.println(F("\n--- Reading SEN0161 (pH) ---"));
  for (int i = 0; i < NUM_SAMPLES; i++){ 
    int16_t rawValue = ads.readADC_SingleEnded(PH_CHANNEL);
    
    // DFRobot expects the voltage in mV, hence multiply by 1000
    float mV_value = ads.computeVolts(rawValue) * 1000.0;

    pHvoltage = mV_value; // EC library expects voltage in mV
    
    // Convert voltage to pH value with temperature compensation
    pHValue = phSensor.readPH(pHvoltage, temp);
    
    phSensor.calibration(pHvoltage, temp); 

    pHReadings[i] = pHValue;
  }

  pHAvg = computeAverage(pHReadings, NUM_SAMPLES);

  printResults("pH", pHReadings, pHAvg, " pH", NUM_SAMPLES);
  

  if (SerialBT.available() > 0) {
    phSensor.calibration(pHvoltage, temp); 
  }
  return pHAvg;
}
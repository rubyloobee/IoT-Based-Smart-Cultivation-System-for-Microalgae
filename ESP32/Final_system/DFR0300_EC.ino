#include "DFRobot_EC.h"        // Convert raw analog voltage to EC value
#include "EEPROM.h"            // Save data into ESP32's non volatile memory
#include "sensor.h"

const byte EC_CALIBRATION_ADDRESS = 0x08;   // EEPROM address to store calibration data (use an unused address)
#define EC_CHANNEL            1  // ADS1115 analog channel A1

DFRobot_EC ECSensor;

float ECReadings[NUM_SAMPLES];
float ECAvg = 0.0;
float ECvoltage = 0.0;
float ECValue = 0.0;

void setupECSensor() {
  // Initialize EC sensor and read calibration data from EEPROM
  ECSensor.begin();
}

float readECSensor(float temp){
  SerialBT.println(F("\n--- Reading DFR0300 (EC) ---"));
  for (int i = 0; i < NUM_SAMPLES; i++){ 
    // Read the raw 16-bit value from the specified channel (EC_CHANNEL/A0)
    int16_t rawValue = ads.readADC_SingleEnded(EC_CHANNEL);
    
    // DFRobot expects the voltage in mV, hence multiply by 1000
    float mV_value = ads.computeVolts(rawValue) * 1000.0;

    ECvoltage = mV_value; 
    
    // Convert voltage to EC value with temperature compensation
    ECValue = ECSensor.readEC(ECvoltage, temp);
    
    // Keep calling calibration() to update internal variables, even when not in calibration mode
    ECSensor.calibration(ECvoltage, temp); 

    ECReadings[i] = ECValue;
  }

  ECAvg = computeAverage(ECReadings, NUM_SAMPLES);

  // Print results
  printResults("EC", ECReadings, ECAvg, " EC", NUM_SAMPLES);
  

  if (SerialBT.available() > 0) {
    ECSensor.calibration(ECvoltage, temp);
  }

  return ECAvg;
}
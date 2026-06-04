#include <Wire.h>               // Required for I2C communication
#include <Adafruit_ADS1X15.h>   // Library for ADS1115
#include "sensor.h"

#define ADS_ADDRESS       0x48  // Default I2C address for ADS1115 (Connect to GND)

// CHANGE ACCORDING TO ESP32 PIN CONNECTION
#define I2C_SDA           23    // ESP32 GPIO pin for I2C SDA
#define I2C_SCL           33    // ESP32 GPIO pin for I2C SCL

Adafruit_ADS1115 ads;

void setupADS1115() {
  // Initialise ADS1115
  if (!ads.begin(ADS_ADDRESS, &Wire)) { 
    SerialBT.println("ADS1115 not found!");
  }
  // Set the Gain/PGA. +/- 4.096V range is ideal for DFRobot sensors (0-3V output)
  ads.setGain(GAIN_TWO); 
}
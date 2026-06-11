#include "DFRobot_PH.h"        // Primary library for pH calculation and calibration
#include "EEPROM.h"            // Used by the pH library for calibration storage

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>               // Required for I2C communication
#include <Adafruit_ADS1X15.h>   // Library for ADS1115 (works for both 1015/1115)

// --- Configuration Pins ---
#define PH_CHANNEL                0   // ADS1115 analog channel A1
#define ADS_ADDRESS             0x48  // Default I2C address for ADS1115 (ADDR connected to GND)
#define PH_CALIBRATION_ADDRESS 0x20   // EEPROM address for pH calibration data

// CHANGE ACCORDING TO PIN CONNECTED TO ESP32
#define I2C_SDA                 15   // ESP32 GPIO pin for I2C SDA
#define I2C_SCL                 4    // ESP32 GPIO pin for I2C SCL
#define ONE_WIRE_BUS          27     // Digital pin for DS18B20 temperature sensor


// --- Sensor Objects ---
DFRobot_PH ph;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

Adafruit_ADS1115 ads;

// --- Variables ---
float voltage, pHValue, temperature = 25.0; // Default temp for initial reading

void setup() {
  Serial.begin(9600);

  EEPROM.begin(32); 

  ph.begin();

  sensors.begin();
  Wire.begin(I2C_SDA, I2C_SCL);

  ads.begin(ADS_ADDRESS);
  // Set the Gain/PGA. This sets the input voltage range. 
  // Gravity pH sensor has output of 0 - 3V
  ads.setGain(GAIN_TWO); 
  
  Serial.println("--- DFRobot pH Sensor V2 Calibration Ready ---");
  Serial.println("Use the Serial Monitor to enter commands:");
  Serial.println(" enterph -> Enter calibration mode");
  Serial.println(" calph -> Calibrate with a standard solution (4.0, 7.0, or 10.0)");
  Serial.println(" exitph -> Save calibration and exit");
  Serial.println("----------------------------------------------");
}

void loop() {
  static unsigned long analogSampleTime = millis();
  static unsigned long tempSampleTime = millis();
  static unsigned long printTime = millis();

  // --- 1. Get Temperature Reading (Every 1 second) ---
  if (millis() - tempSampleTime >= 1000U) {
    tempSampleTime = millis();
    sensors.requestTemperatures();
    
    // CHANGE CALIBRAITON EQUATION
    temperature = sensors.getTempCByIndex(0);
    if (temperature == DEVICE_DISCONNECTED_C) {
      // Fallback to a fixed temperature if sensor fails
      temperature = 22.0; 
    }
  }

  // --- 2. Get Analog Voltage Reading (Every 20ms for stability) ---
  if (millis() - analogSampleTime >= 1000U) {
    analogSampleTime = millis();
    
    int16_t rawValue = ads.readADC_SingleEnded(PH_CHANNEL);
    
    // DFRobot expects the voltage in mV, hence multiply by 1000
    float mV_value = ads.computeVolts(rawValue) * 1000.0;

    voltage = mV_value; 
    
    // Convert voltage to pH value with temperature compensation
    pHValue = ph.readPH(voltage, temperature);
    
    ph.calibration(voltage, temperature); 
  }

  // --- 3. Print Data (Every 1 second) ---
  if (millis() - printTime >= 1000U) {
    printTime = millis();
    Serial.print("T: ");
    Serial.print(temperature, 1);
    Serial.print(" C | V: ");
    // Print voltage in Volts for easier reading
    Serial.print(voltage / 1000.0, 3); 
    Serial.print(" V | pH: ");
    Serial.print(pHValue, 2);
    Serial.println(" ");
  }

  // --- 4. Handle Serial Input for Calibration ---
  if (Serial.available() > 0) {
    // Allows the DFRobot_EC library to process the serial commands (enterph, calph, exitph)
    ph.calibration(voltage, temperature); 
  }
}
#include "DFRobot_EC.h"       // Convert raw analog voltage to EC value
#include "EEPROM.h"           // Save data into ESP32's non volatile memory
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>               // Required for I2C communication
#include <Adafruit_ADS1X15.h>   // Library for ADS1115


// --- Configuration Pins ---
#define EC_CHANNEL            1  // ADS1115 analog channel A1
#define ADS_ADDRESS           0x48  // Default I2C address for ADS1115 (ADDR connected to GND)
#define EC_CALIBRATION_ADDRESS 0x08 // EEPROM address to store calibration data (use an unused address)

// CHANGE ACCORDING TO PIN CONNECTED TO ESP32
#define I2C_SDA                 23    // ESP32 GPIO pin for I2C SDA
#define I2C_SCL                 33    // ESP32 GPIO pin for I2C SCL
#define ONE_WIRE_BUS          4  // Digital pin for DS18B20 temperature sensor
#define ONE_WIRE_BUS          4  // Digital pin for DS18B20 temperature sensor


// --- Sensor Objects ---
DFRobot_EC ec;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

Adafruit_ADS1115 ads; 

// --- Variables ---
float voltage, ecValue, temperature = 25.0; // Default temp for initial reading

void setup() {
  Serial.begin(9600);

  EEPROM.begin(32);
  
  ec.begin();

  sensors.begin();
  Wire.begin(I2C_SDA, I2C_SCL);

  ads.begin(ADS_ADDRESS);
  // Set the Gain/PGA. This sets the input voltage range. 
  // Gravity pH sensor has output of 0 - 3V
  ads.setGain(GAIN_TWO); 
  
  Serial.println("--- DFRobot EC Sensor V2 Calibration Ready ---");
  Serial.println("Use the Serial Monitor to enter commands:");
  Serial.println(" enterec -> Enter calibration mode");
  Serial.println(" calec -> Calibrate with 1413us/cm or 12.88ms/cm solution");
  Serial.println(" exitec -> Save calibration and exit");
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
    temperature = 0.9988 * sensors.getTempCByIndex(0) + 0.0807; 
    temperature = sensors.getTempCByIndex(0);
    if (temperature == DEVICE_DISCONNECTED_C) {
      // Fallback to a fixed temperature if sensor fails
      temperature = 25.0; 
    }
  }

  // --- 2. Get Analog Voltage Reading (Every 20ms for stability) ---
  if (millis() - analogSampleTime >= 1000U) {
    analogSampleTime = millis();

    int16_t rawValue = ads.readADC_SingleEnded(EC_CHANNEL);
    
    // DFRobot expects the voltage in mV, hence multiply by 1000
    float mV_value = ads.computeVolts(rawValue) * 1000.0;

    voltage = mV_value; 
    
    // Convert voltage to EC value with temperature compensation
    ecValue = ec.readEC(voltage, temperature);
    
    ec.calibration(voltage, temperature); 
  }

  // --- 3. Print Data (Every 1 second) ---
  if (millis() - printTime >= 1000U) {
    printTime = millis();
    Serial.print("T: ");
    Serial.print(temperature, 1);
    Serial.print(" C | V: ");
    Serial.print(voltage / 1000.0, 3);
    Serial.print(" V | EC: ");
    Serial.print(ecValue, 4);
    Serial.println(" ms/cm");
  }

  // --- 4. Handle Serial Input for Calibration ---
  if (Serial.available() > 0) {
    // Allows the DFRobot_EC library to process the serial commands (enterec, calec, exitec)
    ec.calibration(voltage, temperature); 
  }
}
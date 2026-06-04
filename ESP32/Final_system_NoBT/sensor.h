# ifndef SENSOR_h
# define SENSOR_h

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include "Adafruit_TCS34725.h"

// CHANGE ACCORDING TO ESP32 PIN CONNECTION
const int ONE_WIRE_BUS_MAIN = 27;    // Main tank temperature sensor
const int ONE_WIRE_BUS_SAMPLE = 14; // Sample tank temperature sensor
const int RXD2 = 16;
const int TXD2 = 17;
const int LED_CONTROL_PIN = 5;
const int TURBIDITY_PIN = 36;

const int NUM_SAMPLES = 10; 
const float ERROR_FLAG = -999.0;
const float ESPADC = 4095.0;                // ESP32's 12-bit ADC range (0 to 4095)
const float ESPVOLTAGE = 3300.0;            // ESP32's operating voltage in mV (3.3V)

extern Adafruit_ADS1115 ads;

void getMainData();
void getSamplingData();

void setupTempSensor();
void setupLightSensor();
void setupUltrasonicSensor();
void setuppHSensor();
void setupECSensor();
void setupColourSensor();
void setupADS1115();


float readTempSensor(int pin);  // Accept pin to know which sensor to read
float readLightSensor(bool print);
float readUltrasonicSensor(bool print);
float readpHSensor(float temp);
float readECSensor(float temp);
float readColourSensor();
float readTurbiditySensor();

void pollSensorFast();
String data_to_json(String timeString, float temp, float lux, float waterLevel, float pH);
String data_to_json(String timeString, float ec);
float computeAverage(float* readings, int size);
void printResults(const char* sensorName, float* readings, float avg, const char* unit, int size);

#endif // SENSOR__H

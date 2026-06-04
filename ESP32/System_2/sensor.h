# ifndef SENSOR_h
# define SENSOR_h

// --- LIBRARY ---
#include "DHT.h"

// --- DHT11 CONSTANTS ---
const int DHTPIN = 4;
#define DHTTYPE DHT11

// --- CONFIGURATION CONSTANTS ---
const float ERROR_FLAG = -999.0;
const int NUM_SAMPLES = 10; 

// --- GET SENSOR DATA & PUBLISH ---
void getMainData();
void getSamplingData();

// --- SENSOR SETUP FUNCTION ---
void setupTempSensor();

// --- SENSOR READING FUNCTION ---
float readTempSensor();  

// --- UTILITY FUNCTION PROTOTYPES ---
String data_to_json(float temp);
float computeAverage(float* readings, int size);
void printResults(const char* sensorName, float* readings, float avg, const char* unit, int size);

#endif // SENSOR__H

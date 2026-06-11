#include <Wire.h>               // I2C protocol
#include <BH1750.h> 

BH1750 lightMeter;

const int NUM_SAMPLES = 10;     // number of readings to average

int readings[NUM_SAMPLES];
int indexCount = 0;
bool filled = false;

void setup() {
  Serial.begin(9600);
  // Using the default pins (GPIO 21 for SDA, GPIO 22 for SCL)
  Wire.begin(); 
  
  // Initialize the BH1750 sensor
  // CONTINUOUS_HIGH_RES_MODE: Higher resolution (0.5 Lux steps), slower measurement time (~ 120ms)
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Sensor Initialized"));
  } else {
    Serial.println(F("Error initialising BH1750!"));
  }
}

void loop() {
  float lux = lightMeter.readLightLevel();

  // Store reading in buffer (10 data values)
  readings[indexCount++] = lux;
  if (indexCount >= NUM_SAMPLES) {
    indexCount = 0;
    filled = true;
  }

  // Only output after buffer is full (Moving average)
  if (filled) {
    printBuffer();
    
    float avg = computeAverage();

    Serial.print("Moving Average Light: ");
    Serial.print(lux);
    Serial.println(" lx");
  }     

  delay(1000);
}

// Computes moving average of stored readings
float computeAverage() {
  long total = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) total += readings[i];
  return (float)total / NUM_SAMPLES;
}

void printBuffer() {
  Serial.print("\nBuffer: [");
  for (int i = 0; i < NUM_SAMPLES; i++) {
    Serial.print(readings[i], 1);
    if (i < NUM_SAMPLES - 1) {
      Serial.print(", ");
    }
  }
  Serial.println("]");
}
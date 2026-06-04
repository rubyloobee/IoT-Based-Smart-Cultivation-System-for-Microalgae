#ifndef HEADER_H
#define HEADER_H

// --- Pin Definitions ---
// CHANGE ACCORDING TO ESP32 PIN CONNECTION
const int I2C_SDA = 19;
const int I2C_SCL = 18;
const int LED_CONTROL_PIN = 4;
const int TURBIDITY_PIN = 34;

const int NUM_SAMPLES = 10;

// Measure distilled water water first, then put that Y value here:
const float Y0 = 2818.1567;

void setSensorLED(bool state);
void setupColourSensor();
void readColourSensor();
void readTurbiditySensor();


#endif
# ifndef ACTUATOR_h
# define ACTUATOR_h

#include <Adafruit_NeoPixel.h>

extern int target_light_intensity;
extern int target_water_level;      

// Sensor reading
extern float lux;
extern float waterLevel;

// CHANGE ACCORDING TO ESP32 CONNECTION
const int M1A = 12;    
const int M1B = 13; 
const int LED_PIN = 23;
const int RELAY_1 = 32;
const int RELAY_2 = 33;
const int RELAY_3 = 25;
const int RELAY_4 = 26;

const int LED_COUNT = 30;       // Number of LEDs

void setupMotor();
void setupLight();
void setupPump();

void updateMotor();
void updatePhotoperiod (const char* status); 
void adjustLightIntensity();
void samplingPump();
void adjustWaterLevel();
void nutrientPump();
void harvestPump();

#endif
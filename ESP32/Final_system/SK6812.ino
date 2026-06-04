# include "actuator.h"

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int currentBrightness = 0; 
bool isLightOn = false; 


void setupLight() {
  strip.begin();
  strip.setBrightness(0);
  strip.show();
}

void updatePhotoperiod(const char* status) {
  if (strcmp(status, "on") == 0) {
    isLightOn = true;
    SerialBT.println("Photoperiod: Light ON");
  } 
  else {
    isLightOn = false;
    currentBrightness = 0;
    strip.setBrightness(0);
    strip.show();
    SerialBT.println("Photoperiod: Light OFF");
  }
  strip.show(); 
}

void adjustLightIntensity() {
  // Don't adjust during photoperiod OFF time
  if (!isLightOn) return;

  // 1. Calculate the Error
  float error = target_light_intensity - lux;

  // 2. Define a Sensitivity/Gain factor
  // Increase this number (e.g., 0.5 or 1.0) for faster response
  // Decrease it (e.g., 0.1) if the light starts "flashing" or oscillating
  float Kp = 0.2; 

  // 3. Calculate Adjustment
  int adjustment = (int)(error * Kp);

  // 4. Apply Adjustment to currentBrightness
  currentBrightness += adjustment;

  // 5. Constrain to 8-bit range (0-255)
  currentBrightness = constrain(currentBrightness, 0, 255);

  if (adjustment != 0) {
    SerialBT.printf("[Light Control] | Lux: %.1f | Target: %d | Adjustment: %d | Brightness: %d\n", 
                     lux, target_light_intensity, adjustment, currentBrightness);
  }

  for(int i=0; i<LED_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(255, 255, 255)); 
  }
  strip.setBrightness(currentBrightness);
  strip.show();
}
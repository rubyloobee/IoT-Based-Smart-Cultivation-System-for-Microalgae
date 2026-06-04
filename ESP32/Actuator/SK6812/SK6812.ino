#include <Adafruit_NeoPixel.h>  // Library to use NeoPixel command

#define LED_PIN     23       // CHANGE ACCORDING TO PIN CONNECTED TO ESP32
#define LED_COUNT   30       // Number of LEDs
#define BRIGHTNESS  100      // 0–255

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();                   
  strip.setBrightness(BRIGHTNESS); 

  // ------- CODE TO SET COLOUR NORMALLY ---------
  // // LED 0 (First LED) - Bright Red
  // // R=255 (Max Red), G=0, B=0
  // strip.setPixelColor(0, strip.Color(255, 0, 0)); 

  // // LED 1 - Medium Bright Green
  // // R=0, G=128 (Half Bright Green), B=0
  // strip.setPixelColor(1, strip.Color(0, 128, 0)); 

  // // LED 2 - Dim Blue
  // // R=0, G=0, B=50 (Very Dim Blue)
  // strip.setPixelColor(2, strip.Color(0, 0, 50)); 

  // // LED 3 - Yellow (Red + Green) at a Medium Brightness
  // // R=150, G=150, B=0
  // strip.setPixelColor(3, strip.Color(150, 150, 0)); 

  // // LED 4 (Last LED) - White (All Colors) at Half Brightness
  // // R=128, G=128, B=128
  // strip.setPixelColor(4, strip.Color(128, 128, 128)); 
  
  

  strip.show(); 
}

void loop() {

  // Varying white colour brightness at 5ms delay
  colorFade(strip.Color(128, 128, 128), 20);

  // // Cycle through colors
  // colorWipe(strip.Color(255, 0, 0), 200); // Red
  // colorWipe(strip.Color(0, 255, 0), 200); // Green
  // colorWipe(strip.Color(0, 0, 255), 200); // Blue
}

// Simple color wipe
void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}

// Fades the entire strip in and out with a single color
void colorFade(uint32_t color, int fadeSpeed) {
  int maxBrightness = BRIGHTNESS;

  for (int b = 0; b <= maxBrightness; b++) {
    strip.setBrightness(b);
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(128, 128, 128));
    }
    strip.show(); 
    delay(fadeSpeed); 
  }

  // FADE OUT (Brightness from BRIGHTNESS down to 0)
  for (int b = maxBrightness; b >= 0; b--) {
    strip.setBrightness(b);  
    strip.show();
    delay(fadeSpeed); 
  }
}
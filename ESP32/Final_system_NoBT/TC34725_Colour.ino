#include "sensor.h"

// CHANGE: Measure distilled water first, then put that Y value here:
const float Y0 = 1300.0;

// Initialize with specific integration time and gain
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_154MS, TCS34725_GAIN_16X);

void setSensorLED(bool state) {
  digitalWrite(LED_CONTROL_PIN, state ? HIGH : LOW);
  // Small delay to let the LED brightness stabilize or discharge
  delay(50); 
}

void setupColourSensor() {
  pinMode(LED_CONTROL_PIN, OUTPUT);

  if (!tcs.begin(0x29, &Wire)){
    Serial.println("TCS34725 not found!");
    while (1);
  }
}

float readColourSensor() {
  uint32_t rSum = 0, gSum = 0, bSum = 0, cSum = 0;
  uint16_t r, g, b, c;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    tcs.getRawData(&r, &g, &b, &c);
    rSum += r; gSum += g; bSum += b; cSum += c;
    delay(160);  // Match integration time
  }

  // Calculate Averages
  float avgR = (float)rSum / NUM_SAMPLES;
  float avgG = (float)gSum / NUM_SAMPLES;
  float avgB = (float)bSum / NUM_SAMPLES;
  float avgC = (float)cSum / NUM_SAMPLES;

  // ITU-R BT.709 Luminance: Y = 0.2126R + 0.7152G + 0.0722B
  float Y_current = (0.2126 * avgR) + (0.7152 * avgG) + (0.0722 * avgB);

  // Beer-Lambert Absorbance: ABSRGB = -ln(I / I0)
  float absorbance = 0;
  if (Y0 > 0 && Y_current > 0) {
    absorbance = -log(Y_current / Y0); 
  }

  // Growth Health
  float greenRatio = (avgG / (avgR + avgG + avgB)) * 100.0;

  Serial.println(F("\n--- Reading TCS34725 (Colour) ---"));
  Serial.print(F("Avg Raw ->  R: ")); Serial.print(avgR, 1);
  Serial.print(F(" | G: ")); Serial.print(avgG, 1);
  Serial.print(F(" | B: ")); Serial.print(avgB, 1);
  Serial.print(F(" | C: ")); Serial.println(avgC, 1);
  Serial.print(F("Luminance (Y): ")); Serial.println(Y_current, 4);
  Serial.print(F("Absorbance:    ")); Serial.println(absorbance, 4);
  Serial.print(F("Chlorophyll %: ")); Serial.print(greenRatio, 2); Serial.println("%");

  return 2.0;
}
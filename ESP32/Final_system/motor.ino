#include "actuator.h"

// PWM settings for ESP32
const int freq = 20000; // 20kHz for quiet operation
const int resolution = 8; // 8-bit resolution (0-255)

void setupMotor() {
  // Attach pins to PWM channels
  ledcAttach(M1A, freq, resolution);
  ledcAttach(M1B, freq, resolution);

  // Force motor to 0 RPM on startup
  ledcWrite(M1A, 0);
  ledcWrite(M1B, 0);
}

// Separate function to handle the pins
void updateMotor() {
  // Constrain the target speed for safety
  int safeRPM = constrain(target_stirring_speed, 0, 200);

  if (safeRPM == 0) {
    ledcWrite(M1A, 0);
    ledcWrite(M1B, 0);
    SerialBT.println("Motor stopped.");
  } else {
    int pwmValue = map(safeRPM, 0, 200, 0, 255);
    ledcWrite(M1B, 0);         // GND side
    ledcWrite(M1A, pwmValue);  // PWM side

    SerialBT.printf("Motor set to %d RPM (PWM: %d)\n", safeRPM, pwmValue);
  }
}
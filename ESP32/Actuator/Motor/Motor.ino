// CHANGE BELOW ACCORDING TO PIN CONNECTED TO ESP32
const int m1a = 12; 
const int m1b = 13;

// PWM settings for ESP32
const int freq = 20000; // 20kHz for quiet operation
const int resolution = 8; // 8-bit resolution (0-255)

// Global variable to "remember" the speed
int currentRPM = 0;

void setup() {
  Serial.begin(9600);
  
  // Attach pins to PWM channels
  ledcAttach(m1a, freq, resolution);
  ledcAttach(m1b, freq, resolution);
}

void loop() {
  // Check if there is actually data waiting
  if (Serial.available() > 0) {
    
    // Read the number. If it's not a number, it returns 0
    int input = Serial.parseInt();

    if (Serial.read() != -1) {
      currentRPM = constrain(input, 0, 200);
      
      Serial.print("Target RPM set to: ");
      Serial.println(currentRPM);
      
      // Apply the change immediately
      updateMotor();
    }
  }
}

// Separate function to handle the pins
void updateMotor() {
  if (currentRPM == 0) {
    ledcWrite(m1a, 0);
    ledcWrite(m1b, 0);
  } else {
    int pwmValue = map(currentRPM, 0, 200, 0, 255);
    ledcWrite(m1b, 0);         // GND side
    ledcWrite(m1a, pwmValue);  // PWM side
  }
}
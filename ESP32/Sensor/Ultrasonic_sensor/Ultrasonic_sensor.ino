#define RXD2 16
#define TXD2 17

const int NUM_SAMPLES = 10;
const float TANK_HEIGHT = 21.97; // CHANGE TO YOUR TANK MAX HEIGHT IN CM

float readings[NUM_SAMPLES];
int indexCount = 0;

bool readSensor(int &distance);
void pollSensor();
float getWaterLevelCm(); 

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
  for(int i=0; i<NUM_SAMPLES; i++) {
    readings[i] = 0.0;
  }
  Serial.println("System Initialized - Reporting Water Level in CM");
}

void loop() {
  // Constantly clear the UART buffer and update the circular array
  pollSensor();

  // Print the data every 500ms
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    float levelCm = getWaterLevelCm();
    
    Serial.print("Current Water Level: ");
    Serial.print(levelCm, 2); 
    Serial.println(" cm");
    
    lastPrint = millis();
  }
}

void pollSensor() {
  int distance_mm = 0;
  if (readSensor(distance_mm)) {
    // Convert mm to cm
    float distance_cm = distance_mm / 10.0;
    
    // CHANGE CALIBRATION EQUATION
    float correctedDistance = 0.9573 * distance_cm - 0.2435;
    
    // Calculate actual Water Level (Height - Distance)
    float correctedWaterLevel = TANK_HEIGHT - correctedDistance;

    // Update Circular Buffer
    readings[indexCount] = correctedWaterLevel;
    indexCount = (indexCount + 1) % NUM_SAMPLES;
  }
}

bool readSensor(int &distance) {
  if (Serial2.available() >= 4) {
    uint8_t header = Serial2.read();
    if (header == 0xFF) {
      uint8_t high = Serial2.read();
      uint8_t low = Serial2.read();
      uint8_t sum = Serial2.read();
      uint8_t checksum = (0xFF + high + low) & 0xFF;

      if (sum == checksum) {
        distance = (high << 8) + low;
        return true; 
      }
    }
  }
  return false; 
}

// Returns the average water height in cm
float getWaterLevelCm() {
  float total = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    total += readings[i];
  }
  float avgCm = total / NUM_SAMPLES;

  // Constrain result between 0 and the actual tank height
  return constrain(avgCm, 0.0, TANK_HEIGHT);
}
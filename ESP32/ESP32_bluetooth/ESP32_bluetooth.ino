#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_Microalgae"); // The name your laptop/mobile phone will see
  
  Serial.println("USB Serial Started!");
  delay(1000);
  SerialBT.println("Bluetooth Serial Started!");
  
  // Test Pin (Built-in LED)
  pinMode(2, OUTPUT); 
}

unsigned long lastBTPrint = 0;

void loop() {
  digitalWrite(2, HIGH); // Blink internal LED to show life
  delay(500);
  digitalWrite(2, LOW);
  delay(500);
  
  if (millis() - lastBTPrint > 2000) { // Every 2 seconds
    if (SerialBT.hasClient()) { // Only print if a laptop/mobile phone is actually connected
      SerialBT.print("Hi"); 
      SerialBT.print("cj7"); 
      SerialBT.println("--------------------");
    }
    lastBTPrint = millis();
  }
}
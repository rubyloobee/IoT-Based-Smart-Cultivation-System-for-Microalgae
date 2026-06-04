#include <ArduinoJson.h>
#include "MQTT.h"
#include "sensor.h"
#include "EEPROM.h" 
#include "actuator.h"
#include "BluetoothSerial.h"

// Create the Bluetooth instance
BluetoothSerial SerialBT;    

// MQTT and Wi-Fi Clients
WiFiClient espClient;
PubSubClient client(espClient);

// Global Control Variables
int target_light_duration = 0;
int target_light_intensity = 0;
int target_stirring_speed = 0;
int target_water_level = 0;

float mainTemp = 0.0;
float sampleTemp = 0.0;
float lux = 0.0;
float waterLevel = 0.0;
float pH = 0.0;
float EC = 0.0;
float absorbanceRGB = 0.0;
float absorbanceTurbidity = 0.0;

// Closed Loop Control Variables
unsigned long lastAdjustmentTime = 0;
const int adjustmentInterval = 100; // Light adjustment every 0.1s
unsigned long lastPumpTime = 0;
const int pumpInterval = 500; // Water level adjustment every 0.5s

void dualPrint(const char* format, ...) {
  char loc_buf[256]; // Buffer for the message
  va_list arg;
  va_start(arg, format);
  vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
  va_end(arg);

  // Print to USB Serial
  Serial.print(loc_buf);

  // Print to Bluetooth only if a phone is connected
  if (SerialBT.hasClient()) {
    SerialBT.print(loc_buf);
  }
}

void setup() {
  Serial.begin(115200);

  delay(1000); // Wait for power to stabilize
  if (!SerialBT.begin("ESP32_Microalgae")) {
    Serial.println("Bluetooth Init Failed!");
  } else {
    Serial.println("Bluetooth Serial Started!");
  }

  // Wi-Fi Setup
  dualPrint("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  dualPrint("\nWiFi connected!");
  dualPrint("IP Address: %s\n", WiFi.localIP().toString().c_str());

  // Initialise time
  initTime(); 
  dualPrint("Time initialise\n");
  
  // Setup MQTT and set handler for incoming messages
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  client.setKeepAlive(60);
  dualPrint("MQTT done\n");

  // Actuator setup
  setupMotor();
  setupLight();
  setupPump();
  dualPrint("actuator done\n");

  // Initialise I2C busses
  // Primary bus for ADS1115 and colour sensor
  Wire.begin(15, 4); 
  // Secondary bus for light Sensor
  Wire1.begin(21, 22);

  // ADS1116 setup
  EEPROM.begin(64);
  setupADS1115();


  // Sensor setup
  setupTempSensor();
  setupLightSensor();
  setupUltrasonicSensor();
  setuppHSensor();
  setupECSensor();
  setupColourSensor();


  // Run pollSensorFast() enough times to replace all initial zero values in waterLevelReadings
  for (int i = 0; i < 20; i++) { // Call it 20 times over the 1 second
    pollSensorFast();
    delay(50); // Match sensor output rate (transmission of 4 bytes = 50ms)
  }

  dualPrint("--- Initialization Complete ---\n");
}

void loop() {
  // If client is disconnected from MQTT broker, reconnect
  if (!client.connected()) reconnect();
  client.loop(); 

  // Poll the fast-reading sensor (A02YYUW) constantly
  pollSensorFast();

  // --- Light Intensity Control (100ms) ---
  if (millis() - lastAdjustmentTime > adjustmentInterval) {
    // Get light intensity reading
    lux = readLightSensor(false); 
    
    // Adjust SK6812 based on that reading
    adjustLightIntensity();
    
    lastAdjustmentTime = millis();
  }

  // --- Water Level Control (500ms) ---
  if (millis() - lastPumpTime > pumpInterval) {
    // Get water level reading
    waterLevel = readUltrasonicSensor(false); 
    
    // Adjust water level using pump
    adjustWaterLevel();
    
    lastPumpTime = millis();
  }


}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(CLIENT_ID)) {
      subscribe_to_topic(TOPIC_CONTROL);
      subscribe_to_topic(TOPIC_LOG);
      subscribe_to_topic(TOPIC_ACTION);

    } else {
      dualPrint("failed, rc=%d\n", client.state());
      dualPrint("try again in 5 seconds\n");
      delay(5000);
    }
  }
}
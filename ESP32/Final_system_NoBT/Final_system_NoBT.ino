#include <ArduinoJson.h>
#include "MQTT.h"
#include "sensor.h"
#include "EEPROM.h" 
#include "actuator.h"

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


void setup() {
  Serial.begin(115200);

  // Wi-Fi Setup
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialise time
  initTime(); 
  
  // Setup MQTT and set handler for incoming messages
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  client.setKeepAlive(60);

  // Actuator setup
  setupMotor();
  setupLight();
  setupPump();

  // Initialise I2C busses
  // Primary bus for ADS1115 and colour sensor
  Wire.begin(15, 4); 
  // Secondary bus for light Sensor
  Wire1.begin(21, 22);

  // Sensor setup
  // ADS1116 setup
  EEPROM.begin(64);
  setupADS1115();

  // Call the setup functions from the separate sensor modules
  setupTempSensor();
  setupLightSensor();
  setupUltrasonicSensor();
  setuppHSensor();
  setupECSensor();
  setupColourSensor();

  // Run pollSensorFast() enough times to replace all initial zero values in waterLevelReadings
  // Eliminate condition where averaging function calculate from unitiliazed array
  for (int i = 0; i < 20; i++) { // Call it 20 times over the 1 second
    pollSensorFast();
    delay(50); // Match sensor output rate (tranmission of 4 bytes = 50ms)
  }

  Serial.println(F("--- Initialization Complete ---"));
}

void loop() {
  // If client is disconnected from MQTT broker, reconnect
  if (!client.connected()) reconnect();
  (client.loop);

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
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
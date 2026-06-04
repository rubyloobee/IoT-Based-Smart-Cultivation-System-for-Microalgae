#include <ArduinoJson.h>
#include "MQTT.h"
#include "sensor.h"
// MQTT and Wi-Fi Clients
WiFiClient espClient;
PubSubClient client(espClient);

int target_light_duration = 0;
int target_light_intensity = 0;
int target_stirring_speed = 0;
int target_water_level = 0;

float temp = 0.0;
float lux = 0.0;
float waterLevel = 0.0;
float pH = 0.0;
float EC = 0.0;

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

  // Call the setup functions from the separate sensor modules
  setupTempSensor();

  Serial.println(F("--- Initialization Complete ---"));
}

void loop() {
  // If client is disconnected from MQTT broker, reconnect
  if (!client.connected()) reconnect();
  client.loop(); // Send keep-alive pings, checking for incoming packets, maintain connection state
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(CLIENT_ID)) {
      subscribe_to_topic(TOPIC_CONTROL);
      subscribe_to_topic(TOPIC_LOG);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
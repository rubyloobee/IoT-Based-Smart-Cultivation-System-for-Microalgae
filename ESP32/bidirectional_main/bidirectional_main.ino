#include <WiFi.h>         // connect to ESP32 of Wi-FI network
#include <PubSubClient.h> // allows publishing and subscribing to MQTT broker
#include "time.h"         // Network Time Protocol (NTP) synchronisation
#include "config.h"         // Includes all constants

extern WiFiClient espClient; 
extern PubSubClient client;   

extern void initTime();
extern void callback(char* topic, byte* payload, unsigned int length);
extern void subscribe_to_topic();
extern void publish_status();


// MQTT and Wi-Fi Clients
WiFiClient espClient;
PubSubClient client(espClient);

// --- Reconnection Logic (Critical for both roles) ---
void reconnect() {
  // Runs continuously as long as the MQTT client is not connected to the broker
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Attempt to connect to broker using configured server details
    if (client.connect(CLIENT_ID)) {
      Serial.println("MQTT reconnected!");

      // 1. Re-SUBSCRIBE: Restore the ability to receive commands
      subscribe_to_topic();

    } else {
      // Connection fails, print connection state
      Serial.print("MQTT connection failed, reason code =");
      Serial.print(client.state());
      Serial.println("Trying again in 5 seconds");

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// --- Setup ---
void setup() {
  Serial.begin(9600);
  Serial.println("Connecting to WiFi...");

  // Connects to hotspot/router
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 

  // WiFI.status return the connection status, WL_CONNECTED assigned when connected to a WiFi network
  // while loop keeps checking until ESP32 connects to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  initTime();

  // Configure MQTT and set handler for incoming messages
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);

  Serial.println("Connecting to MQTT...");
  // Initial connection attempt
  if (client.connect(CLIENT_ID)) {
    Serial.println("MQTT connected!");
    subscribe_to_topic();
  } else {
    Serial.print("MQTT connection failed, reason code =");
    Serial.println(client.state());
  }
}

// --- Main Loop ---
void loop() {
  // 1. Connection Maintenance: if client is disconnected from MQTT broker, reconnect
  if (!client.connected()) {
    reconnect(); // Call function from mqtt_handlers.ino
  }
  client.loop(); // Handles background traffic AND triggers the callback
                 // Send keep-alive pings, checking for incoming packets, maintain connection state

  // 2. Publisher Logic (Send Status every 5 second / 5000 milliseconds)
  publish_status();
}

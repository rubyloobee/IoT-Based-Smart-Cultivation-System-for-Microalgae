#ifndef MQTT_h
#define MQTT_h
#include <WiFi.h>
#include <PubSubClient.h>

// Wi-Fi credentials
const char* WIFI_SSID = "Galaxy A55 5G 8F92";
const char* WIFI_PASSWORD = "sxyw3390";

// Raspberry Pi IP (Mosquitto broker)
const char* MQTT_SERVER = "10.183.145.33";

// ESP32 SUBSCRIBES to system 2 specific topics for actuator commands
const char* TOPIC_CONTROL = "system_2/control"; 
const char* TOPIC_LOG = "system_2/log_sensor"; 
// ESP32 PUBLISHES its data to system topics
const char* TOPIC_PUBLISH_MAIN = "system_2/main_tank_data"; 
const char* TOPIC_PUBLISH_SAMPLE = "system_2/sampling_tank_data"; 
// Unique ID for the ESP32 Client
const char* CLIENT_ID = "ESP32_System_2";

// --- NTP Configuration ---
const char* NTP_SERVER = "pool.ntp.org"; 	
const long 	GMT_OFFSET_SEC = 8 * 3600; 	 	// GMT+8 for Kuala Lumpur/Asia
const int 	DAYLIGHT_OFFSET_SEC = 0; 

// --- MQTT FUNCTIONS --
void callback(char* topic, byte* payload, unsigned int length);
void subscribe_to_topic(const char* subscribe_topic);
void reconnect();

// --- TIME FUNCTIONS ---
void initTime();
String getCurrentTime();

#endif
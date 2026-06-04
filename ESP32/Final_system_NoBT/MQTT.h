#ifndef MQTT_h
#define MQTT_h
#include <WiFi.h>
#include <PubSubClient.h>

// CHANGE TO YOUR HOTSPOT NETWORK (ESP32 CAN ONLY CONNECT TO 2.4GHZ)
const char* WIFI_SSID = "Galaxy A55 5G 8F92";
const char* WIFI_PASSWORD = "sxyw3390";

// CHANGE TO YOUR RASPBERRY IP (Mosquitto broker)
const char* MQTT_SERVER = "10.85.186.33";

const char* TOPIC_CONTROL = "system_1/control"; 
const char* TOPIC_LOG = "system_1/log_sensor"; 
const char* TOPIC_ACTION = "system_1/action";
const char* TOPIC_PUBLISH_MAIN = "system_1/main_tank_data"; 
const char* TOPIC_PUBLISH_SAMPLE = "system_1/sampling_tank_data"; 
const char* CLIENT_ID = "ESP32_System_1";

const char* NTP_SERVER = "pool.ntp.org"; 	
const long 	GMT_OFFSET_SEC = 8 * 3600; 	 	// GMT+8 for Kuala Lumpur/Asia
const int 	DAYLIGHT_OFFSET_SEC = 0; 

void callback(char* topic, byte* payload, unsigned int length);
void subscribe_to_topic(const char* subscribe_topic);
void reconnect();

void initTime();
String getCurrentTime();

#endif
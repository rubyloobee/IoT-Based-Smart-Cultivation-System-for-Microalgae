# ifndef MQTT_h
# define MQTT_h

// --- Network and MQTT Configuration ---

// Wi-Fi credentials
// CHANGE TO YOUR HOTSPOT (ESP32 CAN ONLY CONNECT TO 2.4GHz network)
const char* WIFI_SSID = "Galaxy A55 5G 8F92";
const char* WIFI_PASSWORD = "sxyw3390";

// CHANGE TO YOUR RASPBERRY PI IP (Mosquitto broker)
const char* MQTT_SERVER = "10.56.118.33";

// --- Topics ---
// ESP32 PUBLISHES its status/data to this topic (Pi Subscribes here)
const char* TOPIC_PUBLISH = "status/esp32"; 
// ESP32 SUBSCRIBES to this topic for commands (Pi Publishes here)
const char* TOPIC_SUBSCRIBE = "command/esp32";

// Unique ID for the ESP32 Client (Must be unique across all devices)
const char* CLIENT_ID = "ESP32_BiDir_Client";

// --- NTP Configuration ---
const char* NTP_SERVER = "pool.ntp.org"; 	
const long 	GMT_OFFSET_SEC = 8 * 3600; 	 	// GMT+8 for Kuala Lumpur/Asia
const int 	DAYLIGHT_OFFSET_SEC = 0; 

# endif // MQTT_h



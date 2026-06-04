#include "time.h"         // Network Time Protocol (NTP) synchronisation
#include "MQTT.h"

static char time_buffer [50];

// --- Time Initialization ---
void initTime() {
  // === NTP Setup: Synchronize ESP32 Clock ===
  Serial.println("Synchronizing time...");
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);   // initiates time sync
  
  // Wait until time is set
  struct tm timeinfo;    // declares time structure
  if(!getLocalTime(&timeinfo, 15000)){    // Wait up to 15 seconds for time sync
    Serial.println("Failed to obtain time. Check internet connection.");
  } else {
    Serial.println("Time synchronized successfully.");
  }
}

// --- 2. Current Time Retrieval Function ---
String getCurrentTime() {
    unsigned long now = millis();
    
    // Get current time from synchronized clock
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        // If time lookup fails, use uptime count as fallback
        // Format: "Time Fail (Uptime: 1234567)"
        snprintf (time_buffer, 50, "Time Fail (Uptime: %lu)", now / 1000);
    } else {
        // Format the time as a readable string: YYYY-MM-DD HH:MM:SS
        strftime(time_buffer, 50, "%Y-%m-%d %H:%M:%S", &timeinfo);
    }

    // Return the C-string as an Arduino String
    return String(time_buffer);
}
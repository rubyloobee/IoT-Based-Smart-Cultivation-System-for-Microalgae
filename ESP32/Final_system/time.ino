#include "time.h"         // Network Time Protocol (NTP) synchronisation
#include "MQTT.h"
#include <sys/time.h> // Required for high-resolution time
static char time_buffer [50];

// --- Time Initialization ---
void initTime() {
  // NTP Setup: Synchronise ESP32 Clock
  SerialBT.println("Synchronizing time...");
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);   // initiates time sync
  
  // Wait until time is set
  struct tm timeinfo;    
  if(!getLocalTime(&timeinfo, 15000)){    // Wait up to 15 seconds for time sync
    SerialBT.println("Failed to obtain time. Check internet connection.");
  } else {
    SerialBT.println("Time synchronized successfully.");
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
        strftime(time_buffer, 50, "%Y-%m-%d %H:%M:%S", &timeinfo);
    }

    // Return the C-string as an Arduino String
    return String(time_buffer);
}

// Obtain Raw Unix Timestamp in milliseconds
String getUnixTime() {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) { 
        // Fallback to uptime if RTC is not set
        String uptime = String(millis());
        SerialBT.println("Time Sync Failed! Uptime (ms): " + uptime); // 
        return uptime; 
    }

    // Convert seconds and microseconds to a single millisecond value
    long long total_msec = (long long)tv.tv_sec * 1000LL + (tv.tv_usec / 1000);
    String timeStr = String(total_msec);

    SerialBT.print("ESP32 Raw Unix MS: ");
    SerialBT.println(timeStr); 

    return timeStr; // 
}
#include "time.h"         // Network Time Protocol (NTP) synchronisation
#include "config.h"

char time_buffer[50]; 

long lastMsg = 0;
char msg_buffer[100];

// --- Time Initialization ---
void initTime() {
  // === NTP Setup: Synchronize ESP32 Clock ===
  Serial.println("Synchronizing time...");
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER); 
  
  // Wait until time is set
  struct tm timeinfo;    // declares time structure
  if(!getLocalTime(&timeinfo, 10000)){    // Wait up to 10 seconds for time sync
    Serial.println("Failed to obtain time. Check internet connection.");
  } else {
    Serial.println("Time synchronized successfully.");
  }
}

// --- Publish message ---
void publish_status() {
  long now = millis();

  if (now - lastMsg > 5000) {
    lastMsg = now;

    // Get current time from synchronized clock
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      // If time lookup fails, use uptime count as fallback
      snprintf (time_buffer, 50, "Time Fail (Uptime: %ld)", now / 1000);
    } else {
      strftime(time_buffer, 50, "%Y-%m-%d %H:%M:%S", &timeinfo);
    }

    // Construct the final status message
    snprintf (msg_buffer, 100, "Message from ESP32 at %s", time_buffer);

    Serial.println("\n--- Publish message to Raspberry Pi ---");
    Serial.print("Topic:");
    Serial.println(TOPIC_PUBLISH);
    Serial.print("Payload:");
    Serial.println(msg_buffer);

    // Publish to the status topic (Pi should be subscribed here)
    client.publish(TOPIC_PUBLISH, msg_buffer, false); // <-- Use (topic, payload, retain)
                                                      // QoS 0, Retain=false
  }
}



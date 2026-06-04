#include "time.h"         // Network Time Protocol (NTP) synchronisation
#include "MQTT.h"


// --- Publish message ---
void publish_status(const char* jsonPayload, const char* publish_topic) {

  Serial.println("\n--- Publish message to Raspberry Pi ---");
  Serial.print("Topic:");
  Serial.println(publish_topic);
  Serial.print("Payload:");
  Serial.println(jsonPayload);

  // Publish to the status topic (Pi should be subscribed here)
  client.publish(publish_topic, jsonPayload, false); // <-- Use (topic, payload, retain)
                                                     // QoS 0, Retain=false
  
}



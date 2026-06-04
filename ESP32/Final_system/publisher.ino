#include "time.h"         // Network Time Protocol (NTP) synchronisation
#include "MQTT.h"


// --- Publish message ---
void publish_status(const char* jsonPayload, const char* publish_topic) {

  SerialBT.println("\n--- Publish message to Raspberry Pi ---");
  SerialBT.print("Topic:");
  SerialBT.println(publish_topic);
  SerialBT.print("Payload:");
  SerialBT.println(jsonPayload);

  // Publish to the status topic (Pi should be subscribed here)
  client.publish(publish_topic, jsonPayload, false); // Use (topic, payload, retain)
                                                     // QoS 0, Retain=false
  
}



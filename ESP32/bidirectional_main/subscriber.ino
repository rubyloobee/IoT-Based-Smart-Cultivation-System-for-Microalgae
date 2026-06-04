#include <PubSubClient.h> // allows publishing and subscribing to MQTT broker
#include "config.h"

extern PubSubClient client; 

// --- Utility Function: Subscribe ---
// Ensures the subscription runs whether the connection is new (setup) or restored (reconnect).
void subscribe_to_topic() {
  // Re-SUBSCRIBE: Restore the ability to receive commands
  // QoS 1 ensures the Pi's command is delivered reliably
  if (client.subscribe(TOPIC_SUBSCRIBE, 1)) {
    Serial.print("Subscribed to topic: ");
    Serial.println(TOPIC_SUBSCRIBE);
  } else {
    Serial.print("Failed to subscribe, reason code =");
    Serial.println(client.state());
  }
}

// --- Message Handler (Subscriber Role) ---
// This runs whenever a command is received from the Raspberry Pi.
void callback(char* topic, byte* payload, unsigned int length) {

  Serial.println("\n>>> Message received from Raspberry Pi >>>");
  Serial.print("Topic:");
  Serial.println(TOPIC_SUBSCRIBE);

  // Print the payload
  Serial.print("Payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}




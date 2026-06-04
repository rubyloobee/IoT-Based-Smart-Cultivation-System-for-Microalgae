#include <PubSubClient.h> // allows publishing and subscribing to MQTT broker
#include "MQTT.h"
#include "sensor.h"
#include "actuator.h"

void subscribe_to_topic(const char* subscribe_topic) {
  // Re-SUBSCRIBE: Restore the ability to receive commands
  // QoS 1 ensures the Pi's command is delivered reliably
  if (client.subscribe(subscribe_topic, 1)) {
    Serial.print("Subscribed to topic: ");
    Serial.println(subscribe_topic);
  } else {
    Serial.printf("Failed to subscribe to %s, reason code =", subscribe_topic);
    Serial.println(client.state());
  }
}

// --- Message Handler (Subscriber Role) ---
// This runs whenever a command is received from the Raspberry Pi
void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<512> doc;
  deserializeJson(doc, payload, length);
  const char* type = doc["type"] | "";

  // --- Actuator Control ---
  if (strcmp(topic, TOPIC_CONTROL) == 0) {
    Serial.printf("\n[CONTROL] Message on %s\n", topic);

    if (strcmp(type, "FULL_SYNC") == 0) {
      Serial.println("\n[MODE] FULL_SYNC...");
      
      // 1. Assign the new values to global variables
      target_light_duration = doc["target_light_duration"];
      target_light_intensity = doc["target_light_intensity"];
      target_stirring_speed = doc["target_stirring_speed"];
      target_water_level = doc["target_water_level"];

      // 2. Apply actuator update
      updateMotor();
      updatePhotoperiod(lightStatus); 
      
      // 3. Print the full state for verification
      Serial.println("------------------------------------");
      Serial.printf("   >> Target light duration   : %d hrs\n", target_light_duration);
      Serial.printf("   >> Target light intensity : %d Lux\n", target_light_intensity);
      Serial.printf("   >> Target stirring speed: %d RPM\n", target_stirring_speed);
      Serial.printf("   >> Target water level  : %d%%\n", target_water_level);
      Serial.println("------------------------------------");
      Serial.println("FULL SYNC: All variables successfully restored.");
    } 
    else if (strcmp(type, "UPDATE") == 0) {
      Serial.println("[MODE] UPDATE...");

      // Each if-block now updates the variable AND prints the confirmation
      if (doc.containsKey("light_power")) {
        const char* lightStatus = doc["light_power"]; // "on" or "off"
        updatePhotoperiod(lightStatus); 
        Serial.printf("   >> Light Power updated: %s\n", lightStatus);
      }

      if (doc.containsKey("target_light_intensity")) {
        target_light_intensity = doc["target_light_intensity"]; // Assign new value to global variable
        Serial.printf("   >> Target light intensity   : %d lux\n", target_light_intensity);
      }
      
      if (doc.containsKey("target_stirring_speed")) {
        target_stirring_speed = doc["target_stirring_speed"];
        updateMotor();
        Serial.printf("   >> Target stirring speed: %d RPM\n", target_stirring_speed);
      }
      
      if (doc.containsKey("target_water_level")) {
        target_water_level = doc["target_water_level"];
        Serial.printf("   >> Target water level  : %d%%\n", target_water_level);
      }
    }
  }
  // --- Logging Updates ---
  else if (strcmp(topic, TOPIC_LOG) == 0) {
  Serial.printf("\n[LOG] Message on %s\n", topic);

    if (strcmp(type, "get_main_data") == 0) {
      Serial.printf("Message: %s\n", type);
      getMainData();
    } else if (strcmp(type, "get_sampling_data") == 0) {
      Serial.printf("Message: %s\n", type);
      samplingPump();
      getSamplingData();
    }
  }

  else if (strcmp(topic, TOPIC_ACTION) == 0) {
    Serial.printf("\n[ACTION] Message on %s\n", topic);

    if (strcmp(type, "nutrient_start") == 0) {
      Serial.printf("Message: %s\n", type);
      nutrientPump();
    } else if (strcmp(type, "harvest_start") == 0) {
      Serial.printf("Message: %s\n", type);
      harvestPump();
    }
  }

}
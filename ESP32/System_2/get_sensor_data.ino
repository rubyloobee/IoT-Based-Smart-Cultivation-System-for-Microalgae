#include "sensor.h";
#include "MQTT.h"
#include "time.h";

void getMainData() {
  Serial.println("\n=====================================");
    
  // Read sensors
  temp = readTempSensor();  
  lux = 0;
  waterLevel = 0;
  pH = 0;
  
  String mainSensorReadingTime = getCurrentTime();

  // Generate JSON string
  String mainSensor = data_to_json(
    mainSensorReadingTime, 
    temp, 
    lux, 
    waterLevel, 
    pH
  );
  
  Serial.println("\n--- Sensor Data in JSON Format ---");
  Serial.println(mainSensor);

  publish_status(mainSensor.c_str(), TOPIC_PUBLISH_MAIN);

  Serial.println("=====================================");
}

void getSamplingData() {
  Serial.println("\n=====================================");
    
  // Read sensors
  EC = 0;
  
  String sampleSensorReadingTime = getCurrentTime();

  // Generate JSON string
  String sampleSensor = data_to_json(
    sampleSensorReadingTime, 
    EC
  );
  
  Serial.println("\n--- Sensor Data in JSON Format ---");
  Serial.println(sampleSensor);

  publish_status(sampleSensor.c_str(), TOPIC_PUBLISH_SAMPLE);

  Serial.println("=====================================");
}
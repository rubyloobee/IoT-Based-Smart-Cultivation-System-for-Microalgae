#include "sensor.h";
#include "MQTT.h"
#include "time.h";

void getMainData() {
  Serial.println("\n=====================================");
    
  // Read sensors
  mainTemp = readTempSensor(ONE_WIRE_BUS_MAIN);   // Temperature in main tank
  lux = readLightSensor(true);
  waterLevel = readUltrasonicSensor(true);
  pH = readpHSensor(mainTemp);
  
  String mainSensorReadingTime = getCurrentTime();

  // Generate JSON string
  String mainSensor = data_to_json(
    mainSensorReadingTime, 
    mainTemp, 
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
    
  // Conductivity sensor reading
  sampleTemp = readTempSensor(ONE_WIRE_BUS_SAMPLE);   // Temperature in sampling tank
  EC = readECSensor(sampleTemp);

  // Colour sensor reading
  setSensorLED(true);
  absorbanceRGB = readColourSensor();
  setSensorLED(false);

  // Turbidity sensor reading
  absorbanceTurbidity = readTurbiditySensor();

  
  String sampleSensorReadingTime = getCurrentTime();

  // Generate JSON string
  String sampleSensor = data_to_json(
    sampleSensorReadingTime, 
    EC,
    absorbanceRGB,
    absorbanceTurbidity
  );
  
  Serial.println("\n--- Sensor Data in JSON Format ---");
  Serial.println(sampleSensor);

  publish_status(sampleSensor.c_str(), TOPIC_PUBLISH_SAMPLE);

  Serial.println("=====================================");
}
#include "sensor.h";
#include "MQTT.h"
#include "time.h";

void getMainData() {
  SerialBT.println("\n=====================================");
    
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
  
  SerialBT.println("\n--- Sensor Data in JSON Format ---");
  SerialBT.println(mainSensor);
  getUnixTime();
  publish_status(mainSensor.c_str(), TOPIC_PUBLISH_MAIN);

  SerialBT.println("=====================================");
}

void getSamplingData() {
  SerialBT.println("\n=====================================");
    
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
  
  SerialBT.println("\n--- Sensor Data in JSON Format ---");
  SerialBT.println(sampleSensor);
  getUnixTime();
  publish_status(sampleSensor.c_str(), TOPIC_PUBLISH_SAMPLE);

  SerialBT.println("=====================================");
}
#include <ArduinoJson.h>
#include "sensor.h"

// Define the size of the JSON document in memory 
// 256 bytes is a safe size for your 5 data points (timestamp + 4 sensor values)
const size_t JSON_DOC_SIZE = 256; 

// Define a buffer size to hold the float string
const size_t FLOAT_BUFFER_SIZE = 10;

String data_to_json(String timeString, float temp, float lux, float waterLevel, float pH) {
    // 1. Create a StaticJsonDocument on the stack
    // StaticJsonDocument is preferred for efficiency since the size is known
    StaticJsonDocument<JSON_DOC_SIZE> doc;

    // 2. Format Floats Manually and Store as Strings
    char tempStr[FLOAT_BUFFER_SIZE];
    char luxStr[FLOAT_BUFFER_SIZE];
    char waterLevelStr[FLOAT_BUFFER_SIZE];
    char pHStr[FLOAT_BUFFER_SIZE];

    // Use snprintf to convert float to string with "%.2f" precision
    snprintf(tempStr, sizeof(tempStr), "%.2f", temp);
    snprintf(luxStr, sizeof(luxStr), "%.2f", lux);
    snprintf(waterLevelStr, sizeof(waterLevelStr), "%.2f", waterLevel);
    snprintf(pHStr, sizeof(pHStr), "%.2f", pH);

    // 3. Store formatted C-strings into JSON doc
    doc["timestamp"] = timeString;
    doc["temperature_C"] = tempStr;
    doc["light_intensity_lux"] = luxStr;
    doc["water_level_cm"] = waterLevelStr;
    doc["pH_value"] = pHStr;

    // 4. Serialize the JSON document to an Arduino String object
    String jsonString;
    serializeJson(doc, jsonString);

    return jsonString;
}

String data_to_json(String timeString, float EC) {
    // 1. Create a StaticJsonDocument on the stack
    // StaticJsonDocument is preferred for efficiency since the size is known
    StaticJsonDocument<JSON_DOC_SIZE> doc;

    // 2. Format Floats Manually and Store as Strings
    char ECStr[FLOAT_BUFFER_SIZE];

    // Use snprintf to convert float to string with "%.2f" precision
    snprintf(ECStr, sizeof(ECStr), "%.2f", EC);

    // 3. Store formatted C-strings into JSON doc
    doc["timestamp"] = timeString;
    doc["EC_value"] = ECStr;

    // 4. Serialize the JSON document to an Arduino String object
    String jsonString;
    serializeJson(doc, jsonString);

    return jsonString;
}
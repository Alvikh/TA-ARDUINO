void bufferMeasurement() {
  DynamicJsonDocument doc(256);
  doc["device_id"] = clientId;
  doc["temperature"] = isnan(sensorData.temperature) ? 0.0 : sensorData.temperature;
  doc["humidity"] = isnan(sensorData.humidity) ? 0.0 : sensorData.humidity;
  doc["voltage"] = isnan(sensorData.voltage) ? 0.0 : sensorData.voltage;
  doc["current"] = isnan(sensorData.current) ? 0.0 : sensorData.current;
  doc["power"] = isnan(sensorData.power) ? 0.0 : sensorData.power;
  doc["energy"] = isnan(sensorData.energy) ? 0.0 : sensorData.energy;
  doc["frequency"] = isnan(sensorData.frequency) ? 0.0 : sensorData.frequency;
  doc["power_factor"] = isnan(sensorData.power_factor) ? 0.0 : sensorData.power_factor;
  doc["measured_at"] = sensorData.timestamp;

  String jsonStr;
  serializeJson(doc, jsonStr);

  jsonBuffer[bufferIndex] = jsonStr;
  bufferIndex++;

  if (bufferIndex >= 10) {
    sendBufferedData();
    bufferIndex = 0;
  }
}
void sendBufferedData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();

    http.begin(client, "https://pey.my.id/api/energy-measurements");
    http.addHeader("Content-Type", "application/json");

    DynamicJsonDocument doc(4096);
    JsonArray arr = doc.to<JsonArray>();

    for (int i = 0; i < 10; i++) {
      DynamicJsonDocument tempDoc(256);
      deserializeJson(tempDoc, jsonBuffer[i]);
      arr.add(tempDoc);
    }

    String finalJson;
    serializeJson(arr, finalJson);

    int httpResponseCode = http.POST(finalJson);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}


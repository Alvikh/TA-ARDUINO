void bufferMeasurement() {
  DynamicJsonDocument doc(256);
  doc["device_id"] = clientId;  // sesuaikan dengan yang diterima Laravel
  doc["temperature"] = isnan(sensorData.temperature) ? 0.0 : sensorData.temperature;
  doc["humidity"] = isnan(sensorData.humidity) ? 0.0 : sensorData.humidity;
  doc["voltage"] = isnan(sensorData.voltage) ? 0.0 : sensorData.voltage;
  doc["current"] = isnan(sensorData.current) ? 0.0 : sensorData.current;
  doc["power"] = isnan(sensorData.power) ? 0.0 : sensorData.power;
  doc["energy"] = isnan(sensorData.energy) ? 0.0 : sensorData.energy;
  doc["frequency"] = isnan(sensorData.frequency) ? 0.0 : sensorData.frequency;
  doc["power_factor"] = isnan(sensorData.power_factor) ? 0.0 : sensorData.power_factor;
  doc["measured_at"] = sensorData.timestamp;  // format: "d-m-Y H:i:s"

  buffer[bufferIndex] = doc;
  bufferIndex++;

  // Jika sudah 10 data, kirim batch ke server
  if (bufferIndex >= BUFFER_SIZE) {
    sendBufferedData();
    bufferIndex = 0; // reset buffer index
  }
}
void sendBufferedData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure client;  // untuk HTTPS
    client.setInsecure();     // abaikan sertifikat, opsional

    http.begin(client, "https://pey.my.id/api/energy-measurements");
    http.addHeader("Content-Type", "application/json");

    // Bangun array JSON
    DynamicJsonDocument payload(3000);
    JsonArray array = payload.to<JsonArray>();

    for (int i = 0; i < BUFFER_SIZE; i++) {
      array.add(buffer[i]);
    }

    String jsonStr;
    serializeJson(array, jsonStr);

    Serial.println("Sending batch data:");
    Serial.println(jsonStr);

    int httpResponseCode = http.POST(jsonStr);

    if (httpResponseCode > 0) {
      Serial.printf("HTTP Response code: %d\n", httpResponseCode);
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.printf("Failed to send. Error code: %d\n", httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

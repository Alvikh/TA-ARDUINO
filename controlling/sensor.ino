
// Fungsi Handle Sensor Reading
void handleSensorReading() {
  unsigned long now = millis();
  
  // Baca sensor setiap 2 detik
  if (now - lastSensorRead > 2000) {
    lastSensorRead = now;
    readAndPublishSensorData();
  }
}

// Fungsi Read and Publish Sensor Data
void readAndPublishSensorData() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Gagal membaca sensor DHT!");
    return;
  }

  DynamicJsonDocument doc(256);
  doc["id"] = deviceId;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["relay_state"] = relayState ? "ON" : "OFF";
  doc["timestamp"] = millis();

  String jsonStr;
  serializeJson(doc, jsonStr);
  
  client.publish("smartpower/device/sensor", jsonStr.c_str());
  Serial.println("Data sensor terkirim: " + jsonStr);
}
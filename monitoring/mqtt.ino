// MQTT Functions
void publishMQTT() {
  DynamicJsonDocument doc(256);
  doc["id"] = clientId;
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
  Serial.println( jsonStr.c_str());
  mqttClient.publish("iot/monitoring", jsonStr.c_str());
}

void reconnectMQTT() {
  if (mqttClient.connect(clientId.c_str())) {
    // displayMQTTStatus(true);
  }
}



// MQTT Functions
void publishMQTT() {
  DynamicJsonDocument doc(256);
  doc["id"] = clientId;
  doc["temperature"] = sensorData.temperature;
  doc["humidity"] = sensorData.humidity;
  doc["voltage"] = sensorData.voltage;
  doc["current"] = sensorData.current;
  doc["power"] = sensorData.power;
  doc["energy"] = sensorData.energy;
  doc["measured_at"] = sensorData.timestamp;
  
  String jsonStr;
  serializeJson(doc, jsonStr);
  mqttClient.publish("iot/monitoring", jsonStr.c_str());
}

void reconnectMQTT() {
  if (mqttClient.connect(clientId.c_str())) {
    // displayMQTTStatus(true);
  }
}

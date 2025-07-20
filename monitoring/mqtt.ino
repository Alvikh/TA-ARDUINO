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
    mqttClient.subscribe("smartpower/device/control");
    Serial.println("MQTT Connected and Subscribed!");
  } else {
    Serial.println("MQTT Connection Failed.");
  }
}

void initMQTT() {
  mqttClient.setServer("broker.hivemq.com", 1883);
  mqttClient.setCallback(handleMQTTMessage);
}
void handleMQTTMessage(char* topic, byte* payload, unsigned int length) {
  if (firstMessage) {
    firstMessage = false;
    Serial.println("Skip retained message at first subscribe");
    return;
  }
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Payload: ");
  Serial.println(message);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("JSON deserialization failed: ");
    Serial.println(error.c_str());
    return;
  }

  // if (doc["action"] == "esp_control" && doc["state"] == "reset" && doc["id"] == clientId) {
  //   String id = doc["id"] | "";
  //   Serial.println("Reset request received.");
  //   lcd.clear();
  //     drawBorder();
  //   centerText(1, "REMOTE CONTROL");
  //   centerText(2, "RESETTING...");
  //   delay(2000);
  //   ESP.restart();
  // }
}

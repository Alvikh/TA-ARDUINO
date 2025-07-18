void setupMQTT() {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
}
// Fungsi Handle Koneksi MQTT
void handleMQTTConnection() {
  if (!client.connected()) {
    reconnectMQTT();
  }
}

// Fungsi Reconnect MQTT
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(deviceId.c_str())) {
      Serial.println("connected");
      client.subscribe("smartpower/device/control");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Fungsi Callback MQTT
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Parse JSON
  DynamicJsonDocument doc(256);
  deserializeJson(doc, message);
  
  String id = doc["id"];
  String action = doc["action"];
  String state = doc["state"];

  // Validasi ID perangkat
  if (id != deviceId) {
    Serial.println("ID tidak sesuai, abaikan pesan");
    return;
  }

  // Proses kontrol relay
  if (action == "relay_control") {
    if (state == "ON") {
      setRelayState(true);
    } 
    else if (state == "OFF") {
      setRelayState(false);
    }
  }
}

// Fungsi Send Status Acknowledgment
void sendStatusAck(String state) {
  DynamicJsonDocument doc(256);
  doc["id"] = deviceId;
  doc["status"] = "acknowledged";
  doc["state"] = state;
  doc["timestamp"] = millis();

  String jsonStr;
  serializeJson(doc, jsonStr);
  
  client.publish("smartpower/device/status", jsonStr.c_str());
  Serial.println("Status terkirim: " + jsonStr);
}

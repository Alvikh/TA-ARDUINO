void initWiFi() {
  WiFiManager wifiManager;
  WiFiManagerParameter custom_id("id", "Device ID", clientId.c_str(), 32);
  wifiManager.addParameter(&custom_id);
  
  if (!wifiManager.autoConnect("PowerMonitor")) {
    displayError("WiFi Failed");
    ESP.restart();
  }
  
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  clientId = mac;
  
  displayConnectionInfo();
}

void initMQTT() {
  mqttClient.setServer("broker.hivemq.com", 1883);
}
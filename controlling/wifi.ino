
void setupWiFi() {
  WiFiManager wifiManager;
  
  // Uncomment untuk reset settings (debug)
  // wifiManager.resetSettings();
  
  // Set timeout 3 menit
  wifiManager.setTimeout(180);

  if (!wifiManager.autoConnect("SmartPowerController")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupWiFiManager() {
  // Generate device ID terlebih dahulu
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  deviceId = mac;
    String html = "<h1>" + deviceId + "</h1>";
  wifiManager.setCustomHeadElement(html.c_str());

  
  // Buat SSID AP dengan format SmartPower-XXXX (4 karakter terakhir MAC)
  String apSsid = "SmartPower-" + deviceId.substring(8);
  
  wifiManager.setTimeout(180);
  wifiManager.setBreakAfterConfig(true);

  if (!wifiManager.autoConnect(apSsid.c_str())) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
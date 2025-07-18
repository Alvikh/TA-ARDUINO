void setupOTA() {
  // Hostname default adalah esp8266-[ChipID]
  ArduinoOTA.setHostname(("Monitoring-" + WiFi.macAddress().substring(12,17)).c_str());

  // Password OTA (opsional)
  // ArduinoOTA.setPassword("admin123");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
String getLatestVersion() {
  HTTPClient http;
  http.begin(VERSION_URL);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    http.end();
    return payload;
  }
  http.end();
  return "";
}

bool shouldUpdate(String currentVersion, String latestVersion) {
  if (latestVersion == "") return false;
  
  // Simple version comparison (for semantic versioning)
  int currentParts[3], latestParts[3];
  sscanf(currentVersion.c_str(), "%d.%d.%d", &currentParts[0], &currentParts[1], &currentParts[2]);
  sscanf(latestVersion.c_str(), "%d.%d.%d", &latestParts[0], &latestParts[1], &latestParts[2]);

  for (int i = 0; i < 3; i++) {
    if (latestParts[i] > currentParts[i]) return true;
    if (latestParts[i] < currentParts[i]) return false;
  }
  return false;
}

void checkForOTAUpdate() {

  Serial.println("Checking for firmware update...");
  
  String latestVersion = getLatestVersion();
  latestVersion.trim();
  
  Serial.print("Current version: ");
  Serial.print(CURRENT_VERSION);
  Serial.print(", Latest version: ");
  Serial.println(latestVersion);

  if (shouldUpdate(CURRENT_VERSION, latestVersion)) {
    Serial.println("New firmware available, updating...");
      lcd.clear();
  drawBorder();
  centerText(1, "UPDATE VERSION");
  centerText(2, "updating....");
    WiFiClientSecure client;
client.setInsecure(); // abaikan SSL certificate
t_httpUpdate_return ret = httpUpdate.update(client, FIRMWARE_URL);

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", 
                      httpUpdate.getLastError(), 
                      httpUpdate.getLastErrorString().c_str());
        break;
      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;
      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
  } else {
    Serial.println("Already running latest version");
  }
}
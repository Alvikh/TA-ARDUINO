
void initStatusLED() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // Idle state nyala
  ledState = HIGH;
}

void updateLEDIndicator() {
  unsigned long currentMillis = millis();

  if (!wifiConnected) {
    // Mode disconnected - LED nyala terus (idle)
    if (ledState != HIGH) {
      ledState = LOW;
      digitalWrite(LED_PIN, LOW);
    }
  }
  else if (wifiConnected && !mqttConnected) {
    // Mode WiFi connected tapi MQTT belum - BLINK
    if (currentMillis - previousBlinkMillis >= WIFI_BLINK_INTERVAL) {
      previousBlinkMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  }
  else if (mqttConnected) {
    // Mode MQTT connected - LED nyala terus
    if (ledState != HIGH) {
      ledState = HIGH;
      digitalWrite(LED_PIN, HIGH);
    }
  }
}

// Fungsi untuk update status koneksi
void setNetworkStatus(bool wifiStatus, bool mqttStatus) {
  wifiConnected = wifiStatus;
  mqttConnected = mqttStatus;
  
  // Reset blink timer saat status berubah
  previousBlinkMillis = millis();
}


Alert createAlert(String type, String message, String severity) {
  Alert alert;
  alert.type = type;
  alert.message = message;
  alert.severity = severity;
  return alert;
}

void publishAlerts(std::vector<Alert> alerts) {
  for (const auto& alert : alerts) {
    DynamicJsonDocument doc(256);
    doc["device_id"] = clientId;
    doc["type"] = alert.type;
    doc["message"] = alert.message;
    doc["severity"] = alert.severity;
    doc["timestamp"] = sensorData.timestamp;

    String jsonStr;
    serializeJson(doc, jsonStr);
    
    mqttClient.publish("smartpower/device/alert", jsonStr.c_str());
    
    // Tampilkan alert di serial monitor untuk debugging
    Serial.println("Alert Published: " + jsonStr);
    
    // Tampilkan juga di LCD
    displayAlert(alert);
  }
}
bool isSameAlert(Alert a, Alert b) {
  return a.type == b.type && a.message == b.message && a.severity == b.severity;
}

void displayAlert(Alert alert) {
  if (!isSameAlert(alert, previousAlert)) {
    currentAlert = alert;
    alertPending = true;
    alertAlreadyShown = false;
    previousAlert = alert; // simpan untuk perbandingan berikutnya
  }
}
void showAlertOnScreen() {
  lcd.clear();
  drawAlertBorder();
  
  // Line 1: Alert type with icon
  lcd.setCursor(1, 0);
  lcd.print("[!] " + currentAlert.type);
  Serial.println(currentAlert.type+"alert");
  
  // Line 2: Alert message (truncated to fit)
  String displayMessage = currentAlert.message;
  if (displayMessage.length() > 18) {
    displayMessage = displayMessage.substring(0, 18) + "..";
  }
  lcd.setCursor(1, 1);
  lcd.print(displayMessage);
  
  // Line 3: Severity and time
  lcd.setCursor(1, 2);
  lcd.print("Level: " + currentAlert.severity);
  lcd.setCursor(14, 2);
  lcd.print(sensorData.timestamp.substring(11, 16));
  
  // Line 4: Additional info or border
  lcd.setCursor(1, 3);
  lcd.print("-------------------");
}

void drawAlertBorder() {
  // Custom border for alerts
  lcd.setCursor(0, 0);
  lcd.print("\x7E"); // Custom character for alert corner
  for (int i = 1; i < 19; i++) {
    lcd.print("-");
  }
  lcd.print("\x7E");
  
  for (int row = 1; row < 3; row++) {
    lcd.setCursor(0, row);
    lcd.print("|");
    lcd.setCursor(19, row);
    lcd.print("|");
  }
  
  lcd.setCursor(0, 3);
  lcd.print("\x7E");
  for (int i = 1; i < 19; i++) {
    lcd.print("-");
  }
  lcd.print("\x7E");
}
void updateAlertDisplay() {
  unsigned long currentMillis = millis();

  if (isAlertDisplaying) {
    if (currentMillis - lastAlertDisplayTime < alertDisplayDuration) {
      bool blinkState = ((currentMillis / 500) % 2 == 0);

      if (blinkState && !alertCurrentlyVisible) {
        showAlertOnScreen();
                blinkBeep();
        alertCurrentlyVisible = true;
      } else if (!blinkState && alertCurrentlyVisible) {
        lcd.clear();
        alertCurrentlyVisible = false;
      }
    } else {
      // Selesai menampilkan alert
      isAlertDisplaying = false;
      alertCurrentlyVisible = false;
      lastAlertDisplayTime = currentMillis;
      updateDisplay();
    }

  } 
  else if (alertPending && !alertAlreadyShown && 
           (currentMillis - lastAlertDisplayTime >= alertDisplayInterval)) {
    // Tampilkan alert baru hanya jika belum pernah ditampilkan
    isAlertDisplaying = true;
    alertCurrentlyVisible = false;
    alertAlreadyShown = true;
    alertPending = false;
    lastAlertDisplayTime = currentMillis;
    showAlertOnScreen();
  }
}

void checkForAnomalies(SensorData data) {
  // List untuk menyimpan alert yang terdeteksi
  std::vector<Alert> alerts;
  
  // 1. Deteksi tegangan tidak normal
  if (data.voltage < VOLTAGE_UNDER_THRESHOLD) {
    alerts.push_back(createAlert("voltage_under", "Tegangan rendah terdeteksi", "high"));
  } 
  else if (data.voltage > VOLTAGE_OVER_THRESHOLD) {
    alerts.push_back(createAlert("voltage_over", "Tegangan tinggi terdeteksi", "critical"));
  }

  // 2. Deteksi arus berlebih
  if (data.current > CURRENT_OVER_THRESHOLD) {
    alerts.push_back(createAlert("current_over", "Arus berlebih terdeteksi", "critical"));
  }

  // 3. Deteksi daya berlebih
  if (data.power > POWER_OVER_THRESHOLD) {
    alerts.push_back(createAlert("power_over", "Beban daya berlebih", "high"));
  }

  // 4. Deteksi suhu tinggi
  if (data.temperature > TEMP_OVER_THRESHOLD) {
    alerts.push_back(createAlert("temp_over", "Suhu perangkat tinggi", "medium"));
  }

  // 5. Deteksi kelembapan tinggi
  if (data.humidity > HUMIDITY_OVER_THRESHOLD) {
    alerts.push_back(createAlert("humidity_over", "Kelembapan lingkungan tinggi", "low"));
  }

  // Kirim semua alert yang terdeteksi
  if (!alerts.empty()) {
    publishAlerts(alerts);
  }
}
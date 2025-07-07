
// Initialization Functions
void initLCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  displayWelcomeScreen();
}

void displayConnectionInfo() {
  lcd.clear();
  drawBorder();
  centerText(1, "WiFi Connected");
  lcd.setCursor(2, 2);
  lcd.print("Starting System");
  delay(2000);
}

void displayError(String message) {
  lcd.clear();
  drawBorder();
  centerText(1, "ERROR");
  centerText(2, message);
}

void displayMQTTStatus(bool connected) {
  lcd.clear();
  drawBorder();
  centerText(1, "MQTT " + String(connected ? "SUCCESS" : "FAIL"));
  centerText(2, "------");
  
}
void updateDisplay() {
  lcd.clear();
  drawBorder();
  
  switch(currentDisplayPage) {
    case 0:
      // Temperature and Humidity
      centerText(0, "KONDISI");
      lcd.setCursor(2, 1);
      lcd.print("Suhu: " + String(sensorData.temperature, 1) + " C");
      lcd.setCursor(2, 2);
      lcd.print("Kelemb: " + String(sensorData.humidity, 0) + " %");
      break;
      
    case 1:
      // Voltage and Current
      centerText(0, "TEGANGAN & ARUS");
      lcd.setCursor(2, 1);
      lcd.print("Tegangan: " + String(sensorData.voltage, 1) + " V");
      lcd.setCursor(2, 2);
      lcd.print("Arus: " + String(sensorData.current, 2) + " A");
      break;
      
    case 2:
      // Power and Energy
      centerText(0, "DAYA & ENERGI");
      lcd.setCursor(2, 1);
      lcd.print("Daya: " + String(sensorData.power, 1) + " W");
      lcd.setCursor(2, 2);
      lcd.print("Energi: " + String(sensorData.energy, 2) + " kWh");
      break;
      
    case 3:
    Serial.println(sensorData.timestamp);
      // Date and Time (Full format)
      centerText(0, "WAKTU");
      lcd.setCursor(2, 1);
      lcd.print("Tgl: " + sensorData.timestamp.substring(0, 10));
      lcd.setCursor(2, 2);
      lcd.print("Wkt: " + sensorData.timestamp.substring(11));
      break;
  }
}

void createBorderChars() {
  lcd.createChar(0, DOUBLE_VERTICAL);
  lcd.createChar(1, TOP_LEFT_CORNER);
  lcd.createChar(2, TOP_RIGHT_CORNER);
  lcd.createChar(3, BOTTOM_LEFT_CORNER);
  lcd.createChar(4, BOTTOM_RIGHT_CORNER);
}
void drawBorder() {
  // Create custom border characters
  createBorderChars();
  
  // Top border
  lcd.setCursor(0, 0);
  lcd.write(1); // Top-left corner
  for (int i = 1; i < LCD_COLS-1; i++) {
    lcd.print("="); // Horizontal line
  }
  lcd.write(2); // Top-right corner
  
  // Side borders (double vertical lines)
  for (int row = 1; row < LCD_ROWS-1; row++) {
    
    lcd.setCursor(0, row);
    lcd.write(0); // Left double vertical
    lcd.setCursor(LCD_COLS-1, row);
    lcd.write(0); // Right double vertical
  }
  
  // Bottom border
  lcd.setCursor(0, LCD_ROWS-1);
  lcd.write(3); // Bottom-left corner
  for (int i = 1; i < LCD_COLS-1; i++) {
    lcd.print("="); // Horizontal line
  }
  lcd.write(4); // Bottom-right corner
}

void centerText(int row, String text) {
  int col = (LCD_COLS - text.length()) / 2;
  if (col < 0) col = 0;
  lcd.setCursor(col, row);
  lcd.print(text);
}
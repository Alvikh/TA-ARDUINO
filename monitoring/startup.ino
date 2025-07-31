// Display Functions
void displayWelcomeScreen() {
  lcd.clear();
  drawBorder();
  centerText(1, "SMART POWER");
  centerText(2, "MANAGEMENT");
  delay(2000);
  
}
void showWelcomeScreen() {
  lcd.clear();
  for (int i = 0; i < LCD_COLS; i++) {
    // Baris atas
    lcd.setCursor(i, 0);
    lcd.print("=");

    // Baris bawah
    lcd.setCursor(LCD_COLS - 1 - i, 3);
    lcd.print("=");

    delay(30);
  }

  // | | Border samping kiri-kanan
  for (int i = 1; i < 3; i++) {
    lcd.setCursor(0, i);
    lcd.print("|");

    lcd.setCursor(LCD_COLS - 1, i);
    lcd.print("|");

    delay(60);
  }

  // 📝 Judul tengah animasi huruf per huruf
  String title = "SMART POWER";
  int titleStart = (LCD_COLS - title.length()) / 2;

  for (int i = 0; i < title.length(); i++) {
    lcd.setCursor(titleStart + i, 1);
    lcd.print(title[i]);
    delay(80);
  }

  String sub = "MANAGEMENT";
  int subStart = (LCD_COLS - sub.length()) / 2;
  for (int i = 0; i < sub.length(); i++) {
    lcd.setCursor(subStart + i, 2);
    lcd.print(sub[i]);
    delay(60);
  }

  // 🔢 Tampilkan versi & ID perangkat
  String version = "v2.1";
  // lcd.setCursor(LCD_COLS - version.length() - 1, 0);
  // lcd.print(version);

  String mac = WiFi.macAddress();
  mac.replace(":", "");
  String devId = "ID:" + mac;
  lcd.setCursor((LCD_COLS - devId.length()) / 2, 3);
  lcd.print(devId);

  delay(2000); // Tahan tampilan lebih lama
}

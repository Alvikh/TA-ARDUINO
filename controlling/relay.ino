void initHardware() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Relay mati saat startup
  dht.begin();
}
void saveRelayState() {
  EEPROM.write(RELAY_STATE_ADDR, relayState ? 1 : 0);
  EEPROM.commit();
  Serial.println("Relay state saved to EEPROM");
}
void loadRelayState() {
  relayState = EEPROM.read(RELAY_STATE_ADDR) == 1;
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  Serial.print("Loaded relay state from EEPROM: ");
  Serial.println(relayState ? "ON" : "OFF");
}
void generateDeviceId() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  deviceId = mac;
  Serial.print("Device ID: ");
  Serial.println(deviceId);
}
// Fungsi Set Relay State
void setRelayState(bool state) {
  relayState = state;
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  saveRelayState();
  sendStatusAck(state ? "ON" : "OFF");
}

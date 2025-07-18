#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266httpUpdate.h>
#define VERSION_URL "https://raw.githubusercontent.com/Alvikh/TA-ARDUINO/master/controlling/version.md"
#define FIRMWARE_URL "https://raw.githubusercontent.com/Alvikh/TA-ARDUINO/master/controlling/build/esp8266.esp8266.d1_mini/controlling.ino.bin"

#define CURRENT_VERSION "1.0.1"
// Konfigurasi Perangkat
#define RELAY_PIN D2               // Pin untuk relay
#define DHT_PIN D7               // Pin untuk DHT11
#define DHT_TYPE DHT11

// Konfigurasi EEPROM
#define EEPROM_SIZE 64
#define RELAY_STATE_ADDR 0

// Konfigurasi MQTT
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
WiFiManager wifiManager;

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHT_PIN, DHT_TYPE);

// Variabel status
bool relayState = false;
String deviceId;
unsigned long lastMsgTime = 0;
unsigned long lastSensorRead = 0;
void setup() {
  Serial.begin(115200);
  
  initHardware();
  initEEPROM();
  loadRelayState();
    setupWiFiManager();
  generateDeviceId();
  setupMQTT();
    setupOTA();
    checkForOTAUpdate();

}

void loop() {
  handleMQTTConnection();
  client.loop();
    ArduinoOTA.handle();
  handleSensorReading();
}

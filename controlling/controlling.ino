#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
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
  
  // Setup WiFiManager dengan Device ID
  setupWiFiManager();
  
  generateDeviceId();
  setupMQTT();
    setupOTA();

}

void loop() {
  handleMQTTConnection();
  client.loop();
    ArduinoOTA.handle();
  handleSensorReading();
}
void setupOTA() {
  // Hostname default adalah esp8266-[ChipID]
  ArduinoOTA.setHostname(("SmartPlug-" + WiFi.macAddress().substring(12,17)).c_str());

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
// Fungsi Inisialisasi Hardware
void initHardware() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Relay mati saat startup
  dht.begin();
}

// Fungsi Inisialisasi EEPROM
void initEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
}

// Fungsi untuk menyimpan state relay ke EEPROM
void saveRelayState() {
  EEPROM.write(RELAY_STATE_ADDR, relayState ? 1 : 0);
  EEPROM.commit();
  Serial.println("Relay state saved to EEPROM");
}

// Fungsi untuk memuat state relay dari EEPROM
void loadRelayState() {
  relayState = EEPROM.read(RELAY_STATE_ADDR) == 1;
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  Serial.print("Loaded relay state from EEPROM: ");
  Serial.println(relayState ? "ON" : "OFF");
}

// Fungsi Setup WiFi dengan WiFiManager
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

// Fungsi Generate Device ID dari MAC Address
void generateDeviceId() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  deviceId = mac;
  Serial.print("Device ID: ");
  Serial.println(deviceId);
}

// Fungsi Setup MQTT
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

// Fungsi Set Relay State
void setRelayState(bool state) {
  relayState = state;
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  saveRelayState();
  sendStatusAck(state ? "ON" : "OFF");
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

// Fungsi Handle Sensor Reading
void handleSensorReading() {
  unsigned long now = millis();
  
  // Baca sensor setiap 2 detik
  if (now - lastSensorRead > 2000) {
    lastSensorRead = now;
    readAndPublishSensorData();
  }
}

// Fungsi Read and Publish Sensor Data
void readAndPublishSensorData() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Gagal membaca sensor DHT!");
    return;
  }

  DynamicJsonDocument doc(256);
  doc["id"] = deviceId;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["relay_state"] = relayState ? "ON" : "OFF";
  doc["timestamp"] = millis();

  String jsonStr;
  serializeJson(doc, jsonStr);
  
  client.publish("smartpower/device/sensor", jsonStr.c_str());
  Serial.println("Data sensor terkirim: " + jsonStr);
}
void setupWiFiManager() {
  // Generate device ID terlebih dahulu
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  deviceId = mac;

  // Buat parameter custom untuk menampilkan Device ID
  WiFiManagerParameter custom_device_id("deviceid", "Device ID", deviceId.c_str(), 12);
  wifiManager.addParameter(&custom_device_id);
  
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
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PZEM004Tv30.h>
#include <DHT.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <Arduino.h>
#include <driver/ledc.h>
#include "pitches.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>
#define VERSION_URL "https://raw.githubusercontent.com/Alvikh/TA-ARDUINO/master/monitoring/version.md"
#define FIRMWARE_URL "https://raw.githubusercontent.com/Alvikh/TA-ARDUINO/master/monitoring/build/esp32.esp32.esp32/monitoring.ino.bin"

#define CURRENT_VERSION "1.0.4"

// Hardware Configuration
#define DHTPIN 5
#define DHTTYPE DHT22
#define LCD_COLS 20
#define LCD_ROWS 4

#define BUZZER_PIN 27

// Durasi not (ms)
#define NOTE_DURATION 200
#define PAUSE_BETWEEN_NOTES 200

// Timing Configuration
#define SENSOR_READ_INTERVAL 2000
#define DISPLAY_CHANGE_INTERVAL 5000
#define MQTT_RECONNECT_INTERVAL 5000

// Threshold untuk deteksi anomali
#define VOLTAGE_UNDER_THRESHOLD 180.0    // Tegangan rendah (V)
#define VOLTAGE_OVER_THRESHOLD 250.0     // Tegangan tinggi (V)
#define CURRENT_OVER_THRESHOLD 15.0      // Arus berlebih (A)
#define POWER_OVER_THRESHOLD 3500.0      // Daya berlebih (W)
#define TEMP_OVER_THRESHOLD 50.0         // Suhu tinggi (°C)
#define HUMIDITY_OVER_THRESHOLD 80.0     // Kelembapan tinggi (%)
#define FREQUENCY_DEVIATION 2.0          // Deviasi frekuensi (Hz)
// Konfigurasi LED
#define LED_PIN 17
#define WIFI_BLINK_INTERVAL 500  // interval kedip WiFi dalam ms
bool firstMessage = true;
unsigned long previousBlinkMillis = 0;
bool ledState = LOW;
bool wifiConnected = false;
bool mqttConnected = false;
// Add these global variables
unsigned long lastAlertDisplayTime = 0;
unsigned long alertDisplayDuration = 2000; // 2 seconds
unsigned long alertDisplayInterval = 10000; // 10 seconds
bool isAlertDisplaying = false;
byte DOUBLE_VERTICAL[8] = {0b11011, 0b11011, 0b11011, 0b11011, 
                           0b11011, 0b11011, 0b11011, 0b11011};

byte TOP_LEFT_CORNER[8] = {0b11111, 0b11000, 0b11000, 0b11000,
                           0b11000, 0b11000, 0b11000, 0b11000};

byte TOP_RIGHT_CORNER[8] = {0b11111, 0b00011, 0b00011, 0b00011,
                            0b00011, 0b00011, 0b00011, 0b00011};

byte BOTTOM_LEFT_CORNER[8] = {0b11000, 0b11000, 0b11000, 0b11000,
                              0b11000, 0b11000, 0b11000, 0b11111};

byte BOTTOM_RIGHT_CORNER[8] = {0b00011, 0b00011, 0b00011, 0b00011,
                               0b00011, 0b00011, 0b00011, 0b11111};


// Global Objects
LiquidCrystal_I2C lcd(0x27, LCD_COLS, LCD_ROWS);
PZEM004Tv30 pzem(Serial2, 32, 33);
DHT dht(DHTPIN, DHTTYPE);
RTC_DS3231 rtc;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Global Variables
String clientId;
unsigned long lastSensorRead = 0;
unsigned long lastDisplayChange = 0;
unsigned long lastMqttReconnectAttempt = 0;
int currentDisplayPage = 0;
bool alertCurrentlyVisible = false;
bool alertPending = false;         // Ada alert baru yang belum ditampilkan
bool alertAlreadyShown = false;    // Untuk mencegah ditampilkan ulang

bool ledc_initialized = false;
String jsonBuffer[10]; 
int bufferIndex = 0;
// Sensor Data Structure
struct SensorData {
  float temperature;
  float humidity;
  float voltage;
  float current;
  float frequency;
  float power_factor;
  float power;
  float energy;
  String timestamp;
} sensorData;
struct Alert {
  String type;
  String message;
  String severity;
} alert;
Alert currentAlert;
Alert previousAlert;

void initLEDC() {
  if (!ledc_initialized) {
    ledc_timer_config_t timer_conf = {
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .duty_resolution = LEDC_TIMER_8_BIT,
      .timer_num = LEDC_TIMER_0,
      .freq_hz = 2000, // Default frequency, will be changed for each note
      .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t channel_conf = {
      .gpio_num = BUZZER_PIN,
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .channel = LEDC_CHANNEL_0,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = LEDC_TIMER_0,
      .duty = 0,
      .hpoint = 0
    };
    ledc_channel_config(&channel_conf);
    
    ledc_initialized = true;
  }
}

void tones(uint8_t pin, unsigned int frequency) {
  initLEDC();
  ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, frequency);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 127); // 50% duty cycle
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void noTone(uint8_t pin) {
  if (ledc_initialized) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
  }
}
// Bunyi bip pendek (misal: tombol ditekan)
void beep(uint16_t freq = NOTE_C6, uint16_t duration = 100) {
  tones(BUZZER_PIN, freq);
  delay(duration);
  noTone(BUZZER_PIN);
}

// Blink-blink cepat (seperti error, alert, dsb)
void blinkBeep(int times = 3, uint16_t freq = NOTE_D5, uint16_t duration = 100, uint16_t pause = 100) {
  for (int i = 0; i < times; i++) {
    tones(BUZZER_PIN, freq);
    delay(duration);
    noTone(BUZZER_PIN);
    delay(pause);
  }
}
void appInitTone() {
  uint16_t introMelody[] = {
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_B5, NOTE_C6
  };
  uint16_t durations[] = {
    150, 150, 150, 150, 300
  };

  for (int i = 0; i < 5; i++) {
    tones(BUZZER_PIN, introMelody[i]);
    delay(durations[i]);
    noTone(BUZZER_PIN);
    delay(50); // jeda antar nada
  }
}

// Efek naik (startup atau berhasil)
void successTone() {
  uint16_t scale[] = { NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6 };
  for (int i = 0; i < 4; i++) {
    tones(BUZZER_PIN, scale[i]);
    delay(100);
    noTone(BUZZER_PIN);
    delay(50);
  }
}

// Efek turun (gagal/error)
void errorTone() {
  uint16_t scale[] = { NOTE_C6, NOTE_G5, NOTE_E5, NOTE_C5 };
  for (int i = 0; i < 4; i++) {
    tones(BUZZER_PIN, scale[i]);
    delay(100);
    noTone(BUZZER_PIN);
    delay(50);
  }
}
void setup() {
  Serial.begin(115200);
  Serial.println("starting sistem");
  initLCD();
  initSensors();
  initWiFi();
   showWelcomeScreen();
  successTone();
  initMQTT();
    initStatusLED();
      if (!MDNS.begin("esp32")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
      setupOTA();
initLEDC();
  checkForOTAUpdate();
  Serial.println("done setup");
}

void loop() {
  unsigned long currentMillis = millis();
    ArduinoOTA.handle();

  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = currentMillis;
    readSensors();
    publishMQTT();
    checkForAnomalies(sensorData);
    bufferMeasurement();
  }

if (!isAlertDisplaying && currentMillis - lastDisplayChange >= DISPLAY_CHANGE_INTERVAL) {
  lastDisplayChange = currentMillis;
  currentDisplayPage = (currentDisplayPage + 1) % 4;
  updateDisplay();
}

  if (!mqttClient.connected() && 
      currentMillis - lastMqttReconnectAttempt >= MQTT_RECONNECT_INTERVAL) {
    lastMqttReconnectAttempt = currentMillis;
    // errorTone();
    reconnectMQTT();
  } else {
    mqttClient.loop();
  }
  bool wifiStatus = WiFi.status() == WL_CONNECTED;
  bool mqttStatus = mqttClient.connected();
  updateAlertDisplay(); 
  setNetworkStatus(wifiStatus, mqttStatus);
  updateLEDIndicator();
}


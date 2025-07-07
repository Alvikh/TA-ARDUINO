#include <Arduino.h>

#define BUZZER_PIN 34
#define LED_PIN 2  // Onboard LED

// Konfigurasi LEDC (LED Control)
#define LEDC_CHANNEL 0
#define LEDC_RESOLUTION 8
#define LEDC_BASE_FREQ 5000

// Melodi opening
int melody[] = {
  440, 440, 0, 440, 0, 349, 440, 0, 523, 0, 392, 0,
  349, 392, 0, 294, 392, 440, 415, 392, 349, 440, 523, 587,
  0, 349, 523, 440, 349, 392, 330,
  659, 659, 0, 659, 0, 523, 659, 0, 784, 0, 392, 0,
  523, 392, 0, 330, 440, 494, 466, 440, 392, 659, 784, 880,
  0, 587, 784, 659, 523, 587, 494
};

// Durasi masing-masing nada
int noteDurations[] = {
  8, 8, 8, 8, 8, 8, 8, 8, 4, 4, 4, 4,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4,
  4, 8, 8, 8, 8, 8, 4,
  8, 8, 8, 8, 8, 8, 8, 8, 4, 4, 4, 4,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4,
  4, 8, 8, 8, 8, 8, 4
};

void setup() {
  pinMode(LED_PIN, OUTPUT);
  
  // Setup LEDC untuk buzzer
  ledcSetup(LEDC_CHANNEL, LEDC_BASE_FREQ, LEDC_RESOLUTION);
  ledcAttachPin(BUZZER_PIN, LEDC_CHANNEL);
}

void playTone(int frequency, int duration) {
  if (frequency == 0) {
    // Jika frekuensi 0, istirahat (tidak ada suara)
    ledcWriteTone(LEDC_CHANNEL, 0);
    digitalWrite(LED_PIN, LOW);
    delay(duration);
  } else {
    // Mainkan nada dengan frekuensi tertentu
    ledcWriteTone(LEDC_CHANNEL, frequency);
    digitalWrite(LED_PIN, HIGH);
    delay(duration);
    ledcWriteTone(LEDC_CHANNEL, 0);
    digitalWrite(LED_PIN, LOW);
  }
  // Jeda singkat antar nada
  delay(duration * 0.3);
}

void playMelody() {
  int tempo = 900;  // Kecepatan melodi (semakin kecil semakin cepat)
  int melodySize = sizeof(melody) / sizeof(melody[0]);
  
  for (int i = 0; i < melodySize; i++) {
    int noteDuration = tempo / noteDurations[i];
    playTone(melody[i], noteDuration);
  }
}

void loop() {
  playMelody();
  delay(2000);  // Jeda 2 detik sebelum mengulang
}
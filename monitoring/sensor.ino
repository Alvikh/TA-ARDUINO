

void initSensors() {
  dht.begin();
  
  if (!rtc.begin()) {
    displayError("RTC Error!");
    while (true);
  }
  
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

// Sensor Functions
void readSensors() {
  sensorData.temperature = dht.readTemperature();
  sensorData.humidity = dht.readHumidity();
  sensorData.voltage = pzem.voltage();
  sensorData.current = pzem.current();
  sensorData.power = pzem.power();
  sensorData.energy = pzem.energy();
  
  DateTime now = rtc.now();
  sensorData.timestamp = String(now.year()) + "-" + 
                        String(now.month()) + "-" + 
                        String(now.day()) + " " + 
                        String(now.hour()) + ":" + 
                        String(now.minute()) + ":" + 
                        String(now.second());
}

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
    sensorData.power_factor = pzem.pf();
    sensorData.energy = pzem.energy();
    sensorData.frequency = pzem.frequency();
    
    DateTime now = rtc.now();
    
    // Format with leading zeros for values below 10
    char timestamp[20];
    snprintf(timestamp, sizeof(timestamp), 
             "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(),
             now.month(),
             now.day(),
             now.hour(),
             now.minute(),
             now.second());

sensorData.timestamp = String(timestamp);

}
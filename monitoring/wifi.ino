void initWiFi() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  clientId = mac;

  WiFiManager wifiManager;

  // HTML tampil di atas form SSID
  String html = "<h1>" + clientId + "</h1>";
  wifiManager.setCustomHeadElement(html.c_str());

  if (!wifiManager.autoConnect("PowerMonitor")) {
    displayError("WiFi Failed");
    ESP.restart();
  }

  displayConnectionInfo();
}

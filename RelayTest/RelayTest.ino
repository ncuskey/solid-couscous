#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// --- CONFIG ---
const char* ssid = "Mickey Mouse Club House";
const char* password = "sunnyjax1787";
#define RELAY_PIN 5  // D1
#define LED_PIN 4    // D2 (Onboard LED usually, or external strip)

void setup() {
  Serial.begin(115200);
  Serial.println("Booting RelayTest...");
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Off? (Active Low)

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("relay_test_box");
  ArduinoOTA.setPassword("admin");

  ArduinoOTA.onStart([]() { Serial.println("Start"); });
  ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });
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
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  ArduinoOTA.handle();

  static unsigned long lastToggle = 0;
  if (millis() - lastToggle > 2000) {
    lastToggle = millis();
    
    // Toggle
    static bool state = false;
    state = !state;
    
    if (state) {
      // ON (Active Low)
      pinMode(RELAY_PIN, OUTPUT);
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Relay State: ON (Output LOW)");
    } else {
      // OFF (High Impedance / Open Drain simulation)
      pinMode(RELAY_PIN, INPUT);
      Serial.println("Relay State: OFF (Input/Floating)");
    }
  }
}

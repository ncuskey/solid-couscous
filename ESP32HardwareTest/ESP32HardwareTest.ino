/*
 * MINIMAL WiFi Connect Test
 * 
 * Goal: Verify credentials ONLY. No BLE, No NeoPixel, No complex logic.
 */

#include <WiFi.h>

const char* ssid = "Mickey Mouse Club House";
const char* password = "sunnyjax1787";

void setup() {
  Serial.begin(115200);
  delay(2000); // Give time to open monitor
  Serial.println("\n\n--- MINIMAL WIFI CREDENTIAL TEST ---");
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  Serial.print("Connecting to: "); Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start > 15000) {
        Serial.println("\nTIMEOUT! Could not connect.");
        return;
    }
  }
  
  Serial.println("\nSUCCESS! Connected.");
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());
}

void loop() {
  // Do nothing
  delay(1000);
}

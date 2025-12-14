// ----------------------------------------------------------------------
// POWER BANK KEEPER TEST - ACTIVE NETWORK HAMMER
// ----------------------------------------------------------------------
// Strategy 4: High Power + Data Transmission.
// User suggests TX/RX task.
// We will:
// 1. Keep Relay & LED ON (Base Load)
// 2. Set WiFi Output Power to Max
// 3. Flood UDP packets (TX spikes current significantly)

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define RELAY_PIN 5 
const char* ssid = "Mickey Mouse Club House";
const char* password = "sunnyjax1787";

WiFiUDP udp;
// Broadcast IP (send to everyone on subnet)
IPAddress broadcastIP(255, 255, 255, 255);
const int UDP_PORT = 12345;
char packetBuffer[] = "KEEP_ALIVE_POWER_PACKET_DATA_FLOOD";

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  
  // 1. BASE LOAD (Relay + LED)
  digitalWrite(LED_BUILTIN, LOW); 
  digitalWrite(RELAY_PIN, LOW); 
  
  // 2. MAX WIFI POWER (0 - 20.5 dBm)
  WiFi.setOutputPower(20.5); 
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  
  Serial.println("\n\nStarting NETWORK HAMMER mode...");
  Serial.print("Connecting to "); Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi Connected.");
  
  // Calculate broadcast IP based on our subnet just to be clean, 
  // or just use 255.255.255.255 which works for simple cases.
  broadcastIP = IPAddress(255, 255, 255, 255);
  
  udp.begin(UDP_PORT);
}

void loop() {
  // 3. CONTINUOUS TRANSMISSION
  // Sending packets causes current spikes up to ~170-200mA
  udp.beginPacket(broadcastIP, UDP_PORT);
  udp.write(packetBuffer);
  udp.endPacket();
  
  Serial.print("."); // Serial also consumes a tiny bit
  delay(50); // Send 20 packets per second
}

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ----------------------------------------------------------------------
// TODO: Enter your WiFi credentials below!
// ----------------------------------------------------------------------
const char* ssid = "Mickey Mouse Club House";
const char* password = "sunnyjax1787";
// ----------------------------------------------------------------------

ESP8266WebServer server(80);

void handleRoot() {
  String html = "<html><head><title>Christmas Lockbox</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: sans-serif; text-align: center; margin-top: 50px; background-color: #f4f4f4;}";
  html += "h1 { color: #333; }";
  html += ".button { display: inline-block; padding: 15px 25px; font-size: 24px; cursor: pointer; text-align: center; text-decoration: none; outline: none; color: #fff; background-color: #4CAF50; border: none; border-radius: 15px; box-shadow: 0 9px #999; margin: 10px;}";
  html += ".button:hover {background-color: #3e8e41}";
  html += ".button:active {background-color: #3e8e41; box-shadow: 0 5px #666; transform: translateY(4px);}";
  html += ".off { background-color: #f44336; }";
  html += ".off:hover { background-color: #d32f2f; }";
  html += ".off:active { background-color: #d32f2f; }";
  html += "</style></head><body>";
  
  html += "<h1>🎄 Christmas Light Control 🎄</h1>";
  html += "<p>Turn the light On or Off</p>";
  html += "<a href=\"/on\" class=\"button\">ON</a>";
  html += "<a href=\"/off\" class=\"button off\">OFF</a>";
  
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleOn() {
  digitalWrite(LED_BUILTIN, LOW); // Note: On many ESP8266s, LOW is ON (active low)
  Serial.println("LED turned ON via Web");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleOff() {
  digitalWrite(LED_BUILTIN, HIGH); // HIGH is OFF
  Serial.println("LED turned OFF via Web");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Ensure OFF at startup

  Serial.println("\n\nStarting WiFiWebSwitch...");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Wait for connection
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    i++;
    if (i > 30) {
      Serial.println("\nConnection timed out! Check credentials.");
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/on", handleOn);
    server.on("/off", handleOff);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
  }
}

void loop() {
  server.handleClient();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Hello from ESP8266!");
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note: LOW might be ON for some ESPs)
  delay(1000);                      // Wait for a second
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off
  delay(1000);                      // Wait for a second
  Serial.println("Tick");
}

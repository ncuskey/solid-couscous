#include <Adafruit_NeoPixel.h>

#define LED_PIN 13
#define NUM_LEDS 207

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  
  // 1. Init OFF to save power
  strip.begin();
  strip.setBrightness(10); // Super dim
  strip.show(); // Force all Black/Off immediately
  
  Serial.println("Starting Soft LED Test...");
  delay(1000); // Wait for voltage to stabilize
  
  // 2. Light up only 5 LEDs
  Serial.println("Lighting 5 LEDs...");
  for(int i=0; i<5; i++) {
     strip.setPixelColor(i, strip.Color(0, 0, 255)); // Blue
  }
  strip.show();
  delay(1000);
  
  // 3. Light up 20 LEDs
  Serial.println("Lighting 20 LEDs...");
  for(int i=0; i<20; i++) {
     strip.setPixelColor(i, strip.Color(0, 255, 0)); // Green
  }
  strip.show();
}

void loop() {
  // Do nothing
}

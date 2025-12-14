/*
 * HARDWARE TEST SKETCH - PIN CYCLER + NEOPIXEL
 * 
 * 1. Cycles "Simple Pins" HIGH for 2 seconds (measure with Multimeter).
 * 2. Runs a Rainbow Animation on the LED Strip (Pin D2).
 * 
 * PINS TESTED:
 * - D1 (Relay): Toggles HIGH/LOW
 * - D2 (LEDs): Rainbow Swirl
 * - D5, D6, D7, D8: Toggles HIGH/LOW
 */

#include <Adafruit_NeoPixel.h>

// --- CONFIG ---
#define LED_PIN     4   // D2 on Wemos/NodeMCU
#define LED_COUNT   576  // Number of LEDs to test
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Simple Digital Pins to Cycle
int simplePins[] = { 
  5,  // D1 (Relay)
  14, // D5
  12, // D6
  13, // D7
  15  // D8
  // Note: Skipping D3/D4 to avoid boot/serial conflicts during this specific test
};

const char* pinNames[] = {
  "D1 (GPIO 5) -> RELAY",
  "D5 (GPIO 14)",
  "D6 (GPIO 12)",
  "D7 (GPIO 13)",
  "D8 (GPIO 15)"
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n--- HARDWARE & LED TEST ---");

  // Setup Simple Pins
  for (int i = 0; i < 5; i++) {
    pinMode(simplePins[i], OUTPUT);
    digitalWrite(simplePins[i], LOW);
  }

  // Setup LEDs
  strip.begin();
  strip.setBrightness(50); // Moderate brightness
  strip.show(); // Initialize all to 'off'
  Serial.println("LED Strip Initialized on D2");
}

void loop() {
  Serial.println("\n--- STARTING ADVANCED PATTERN CYCLE ---");

  // 1. Cylon / KITT (Red Scanner)
  Serial.println("Pattern: Cylon (KITT)");
  cylon(strip.Color(255, 0, 0), 10, 50, 2); // Color, EyeSize, SpeedDelay, ReturnDelay

  // 2. Meteor Rain (Blue)
  Serial.println("Pattern: Meteor Rain");
  meteorRain(0xff, 0xff, 0xff, 10, 64, true, 30); // r, g, b, meteorSize, decay, randomDecay, speedDelay

  // 3. Breathing (Blue/Cyan)
  Serial.println("Pattern: Breathing");
  breathing(0, 0, 255, 10); // r, g, b, speed

  // 4. Sparkle (White flashes)
  Serial.println("Pattern: Sparkle");
  sparkle(255, 255, 255, 50, 100); // r, g, b, count, speedDelay

  // 5. Fire (Red/Orange/Yellow)
  Serial.println("Pattern: Fire");
  fire(55, 120, 15); // Cooling, Sparking, SpeedDelay

  // 6. Rainbow Cycle (Classic)
  Serial.println("Pattern: Rainbow Swirl");
  rainbow(10);
}

// --- VISUAL EFFECTS HELPERS ---

// 1. CYLON / KITT
void cylon(uint32_t color, int eyeSize, int speedDelay, int returnDelay){
  for(int i = 0; i < strip.numPixels()-eyeSize-2; i++) {
    strip.fill(0, 0, strip.numPixels()); // Clear
    strip.fill(color, i, eyeSize); // Draw Eye
    strip.show();
    delay(speedDelay);
  }
  for(int i = strip.numPixels()-eyeSize-2; i > 0; i--) {
    strip.fill(0, 0, strip.numPixels());
    strip.fill(color, i, eyeSize);
    strip.show();
    delay(speedDelay);
  }
}

// 2. METEOR RAIN
void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int speedDelay) {
  strip.fill(0,0,strip.numPixels());
  for(int i = 0; i < strip.numPixels()+strip.numPixels(); i++) {
    // fade brightness all LEDs one step
    for(int j=0; j<strip.numPixels(); j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );        
      }
    }
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j < strip.numPixels()) && (i-j>=0) ) {
        strip.setPixelColor(i-j, red, green, blue);
      } 
    }
    strip.show();
    delay(speedDelay);
  }
}

void fadeToBlack(int ledNo, byte fadeValue) {
   uint32_t oldColor = strip.getPixelColor(ledNo);
   uint8_t r = (oldColor & 0x00ff0000UL) >> 16;
   uint8_t g = (oldColor & 0x0000ff00UL) >> 8;
   uint8_t b = (oldColor & 0x000000ffUL);
   r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
   g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
   b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
   strip.setPixelColor(ledNo, r,g,b);
}

// 3. BREATHING
void breathing(int r, int g, int b, int wait) {
  float maximumBrightness = 255;
  for (int i = 0; i < 65535; i+=500) { // Cycle
    // Intensity = sin wave
    float intensity = (exp(sin(i/2000.0*PI)) - 0.36787944)*108.0;
    strip.fill(strip.Color(r*intensity/maximumBrightness, g*intensity/maximumBrightness, b*intensity/maximumBrightness));
    strip.show();
    delay(wait);
    if(i > 60000) break; // One cycle mostly
  }
}

// 4. SPARKLE
void sparkle(byte red, byte green, byte blue, int count, int speedDelay) {
  for(int i=0; i<count; i++) {
    int Pixel = random(strip.numPixels());
    strip.setPixelColor(Pixel,red,green,blue);
    strip.show();
    delay(speedDelay);
    strip.setPixelColor(Pixel,0,0,0);
  }
}

// 5. FIRE
void fire(int Cooling, int Sparking, int SpeedDelay) {
  static byte heat[600]; // Ideally match LED_COUNT but using generic size for safety
  int cooldown;
  
  // Run for 300 frames
  for(int frame=0; frame<300; frame++) {
    // Step 1.  Cool down every cell a little
    for( int i = 0; i < strip.numPixels(); i++) {
      cooldown = random(0, ((Cooling * 10) / strip.numPixels()) + 2);
      if(cooldown>heat[i]) {
        heat[i]=0;
      } else {
        heat[i]=heat[i]-cooldown;
      }
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= strip.numPixels() - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' near the bottom
    if( random(255) < Sparking ) {
      int y = random(7);
      heat[y] = heat[y] + random(160,255);
    }
  
    // Step 4.  Convert heat to LED colors
    for( int j = 0; j < strip.numPixels(); j++) {
      setPixelHeatColor(j, heat[j] );
    }
  
    strip.show();
    delay(SpeedDelay);
  }
}

void setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    strip.setPixelColor(Pixel, 255, 255, heatramp);
  } else if( t192 > 0x40 ) {             // middle
    strip.setPixelColor(Pixel, 255, heatramp, 0);
  } else {                               // coolest
    strip.setPixelColor(Pixel, heatramp, 0, 0);
  }
}

// 6. RAINBOW
void rainbow(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 65536; firstPixelHue += 256*5) { // Faster loop
    for(int i=0; i<strip.numPixels(); i++) { 
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    delay(wait);
  }
}


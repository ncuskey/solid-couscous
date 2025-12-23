#include <Arduino.h>
#include "AudioGeneratorRTTTL.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourcePROGMEM.h"

// PINS
#define I2S_BCLK 27
#define I2S_LRC  26
#define I2S_DOUT 25
#define AMP_SD   14

AudioGeneratorRTTTL *rtttl;
AudioFileSourcePROGMEM *file;
AudioOutputI2S *out;

// Simple Beep: "Name:d=4,o=5,b=120:c,p,c"
const char beep[] PROGMEM = "Beep:d=4,o=5,b=120:16c,16p,16c,16p,16c";

void setup() {
  Serial.begin(115200);
  Serial.println("Audio Tone Test Starting...");

  // Wake up Amp
  pinMode(AMP_SD, OUTPUT);
  digitalWrite(AMP_SD, HIGH); 

  file = new AudioFileSourcePROGMEM(beep, strlen(beep));
  out = new AudioOutputI2S();
  out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  out->SetGain(0.1); // LOW VOLUME (10%) for testing
  
  rtttl = new AudioGeneratorRTTTL();
  rtttl->begin(file, out);
}

void loop() {
  if (rtttl->isRunning()) {
    if (!rtttl->loop()) {
      rtttl->stop();
      Serial.println("Tone Done. Restarting in 2s...");
      delay(2000);
      
      // Replay
      delete file;
      file = new AudioFileSourcePROGMEM(beep, strlen(beep));
      rtttl->begin(file, out);
    }
  }
}

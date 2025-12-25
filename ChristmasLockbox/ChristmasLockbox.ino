#ifdef ESP32
  #include <WiFi.h>
  #include <ESPmDNS.h>
  #include <esp_wifi.h> // For power save modes
  #include <esp_now.h>  // For low-latency sync
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
#endif
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
// #include "audio_assets.h" // REMOVED for Streaming
#include <SPIFFS.h>
#include <AudioFileSourceSPIFFS.h>
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>

// ----------------------------------------------------------------------
// CONFIGURATION
// ----------------------------------------------------------------------
const char* ssid = "Mickey Mouse Club House";
const char* password = "sunnyjax1787";

const char* mqtt_server = "167.172.211.213";
const int mqtt_port = 1883;

// CHANGE THIS FOR EACH BOX: lockbox/1, lockbox/2, lockbox/3
// Box 1 = Sam (CONDUCTOR for ESP-NOW sync)
// Box 2 = Kristine
// Box 3 = Jacob
const char* box_topic_cmd = "lockbox/1/cmd";
const char* box_topic_status = "lockbox/1/status";
const char* client_id = "ChristmasBox1";

// ----------------------------------------------------------------------
// HARDWARE PINS
// ----------------------------------------------------------------------
#define RELAY_PIN 33 // Changed to 33 (Safe & Accessible on Right Side)
#define LED_PIN   13 // Unchanged
#define AMP_SD_PIN 14 // Unchanged (D14 is accessible)
#define NUM_LEDS  207 

// ----------------------------------------------------------------------
// GLOBAL OBJECTS
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// GLOBAL OBJECTS
// ----------------------------------------------------------------------
WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
WiFiUDP udp;
const int UDP_PORT = 4210;

// AUDIO OBJECTS
AudioGeneratorMP3 *mp3;
AudioFileSourceSPIFFS *file;
AudioFileSourceID3 *id3;
AudioOutputI2S *out;

// AUDIO PINS (Left-Side Breadboard Friendly)
#define I2S_BCLK 27
#define I2S_LRC  26
#define I2S_DOUT 25

// ----------------------------------------------------------------------
// GAME STATE
// ----------------------------------------------------------------------
bool locked = true;
int samProgress = 0;   // 0-3
int krisProgress = 0;  // 0-3
int jacobProgress = 0; // 0-3

// Solenoid Control
unsigned long lockPulseStart = 0;
const unsigned long LOCK_PULSE_MS = 5000;

// Finale Synchronization State
bool samReady = false;
bool krisReady = false;
bool jacobReady = false;
bool finaleArmed = false;

// OTA Stability Flag
bool isUpdating = false;

// FORWARD DECLARATIONS
void audioLoop();
void playFileByName(const char* name);
// void playAudio(const char* url); // REMOVED
void stopAudio();


// ----------------------------------------------------------------------
// LIGHTING STATE MACHINE
// ----------------------------------------------------------------------
enum PatternMode {
    MODE_GAME = 0,
    MODE_CYLON = 1,
    MODE_METEOR = 2,
    MODE_BREATHING = 3,
    MODE_SPARKLE = 4,
    MODE_FIRE = 5,
    MODE_RAINBOW = 6,
    MODE_XMAS = 7,
    MODE_SPEAKING = 8,
    MODE_INTRO_BOOT = 10,
    MODE_INTRO_SPEAKING = 11,
    MODE_INTRO_LISTENING = 12,
    MODE_OFF = 99
};

// Center LED for symmetric animations
#define CENTER_LED 103

// Character color (set based on box number during play_intro)
uint8_t charColorR = 255;
uint8_t charColorG = 180;
uint8_t charColorB = 0;

// Intro timing
unsigned long introStartTime = 0;

PatternMode currentPattern = MODE_GAME;
unsigned long lastUpdate = 0;

// Forward Decls
void updateGameLEDs();
void updateCylon();
void updateMeteor();
void updateBreathing();
void updateSparkle();
void updateFire();
void updateRainbow();
void updateXmas();
void updateSpeaking();
void updateIntroBoot();
void updateIntroSpeaking();
void updateIntroListening();

// ----------------------------------------------------------------------
// ESP-NOW SYNC (Low-latency broadcast for synchronized playback)
// ----------------------------------------------------------------------
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast to ALL
volatile bool espNowSyncReceived = false;
volatile bool isConductor = false; // Set to true for Box 1 (Sam)

// Simple sync message structure with scheduled start time
typedef struct {
    uint8_t command;       // 1 = START_INTRO
    uint32_t startAtMs;    // millis() timestamp when ALL boxes should start
} SyncMessage;

// Scheduled playback variables
volatile uint32_t scheduledStartTime = 0;
volatile bool waitingForScheduledStart = false;

// Called when ESP-NOW message received (ESP-IDF v5.5+ signature)
void onEspNowReceive(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len) {
    if (data_len >= sizeof(SyncMessage)) {
        SyncMessage* msg = (SyncMessage*)data;
        if (msg->command == 1) {
            Serial.print("ESP-NOW SYNC: Received START signal! Start at: ");
            Serial.println(msg->startAtMs);
            scheduledStartTime = msg->startAtMs;
            waitingForScheduledStart = true;
        }
    }
}

// Called when ESP-NOW message sent (ESP-IDF v5.5+ signature)
void onEspNowSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
    Serial.print("ESP-NOW Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}
// LOGIC
// ----------------------------------------------------------------------
void setLock(bool state) {
    locked = state;
    if (locked) {
        // Locked state: Solenoid is OFF (Mechanism is mechanically latched when closed)
        digitalWrite(RELAY_PIN, LOW);
    } else {
        // Unlocked state: Pulse Solenoid to release latch
        digitalWrite(RELAY_PIN, HIGH);
        lockPulseStart = millis();
        
        // POWER OPTIMIZATION: Removed White Flash to prevent brownout during solenoid surge
    }
}

void publishStatus() {
    StaticJsonDocument<512> doc;
    
    // Status
    doc["unlocked"] = !locked;
    doc["pattern"] = (int)currentPattern;
    doc["finaleArmed"] = finaleArmed;
    
    // Progress
    JsonObject sam = doc.createNestedObject("sam");
    sam["progress"] = samProgress; sam["done"] = (samProgress>=3); sam["ready"] = samReady;
    
    JsonObject kris = doc.createNestedObject("kris");
    kris["progress"] = krisProgress; kris["done"] = (krisProgress>=3); kris["ready"] = krisReady;
    
    JsonObject jacob = doc.createNestedObject("jacob");
    jacob["progress"] = jacobProgress; jacob["done"] = (jacobProgress>=3); jacob["ready"] = jacobReady;
    
    char buffer[512];
    serializeJson(doc, buffer);
    client.publish(box_topic_status, buffer, true); // Retained message
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    
    // Convert payload to string
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.println(message);

    // DEBUG: Flash Blue on Message
    for(int i=0; i<5; i++) strip.setPixelColor(i, 0, 0, 255);
    strip.show();
    delay(50);
    strip.clear();
    strip.show();

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }

    const char* action = doc["action"];
    if (!action) return;
    
    bool changed = false;

    if (strcmp(action, "solve") == 0) {
        if (currentPattern == MODE_GAME) {
            const char* kid = doc["kid"];
            if (kid) {
                if (strcmp(kid, "sam") == 0 && samProgress < 3) { 
                    samProgress++; 
                    if(samProgress >= 3) setLock(false); // WINNER
                    changed = true; 
                }
                if (strcmp(kid, "kris") == 0 && krisProgress < 3) { 
                    krisProgress++; 
                    if(krisProgress >= 3) setLock(false); // WINNER
                    changed = true; 
                }
                if (strcmp(kid, "jacob") == 0 && jacobProgress < 3) { 
                    jacobProgress++; 
                    if(jacobProgress >= 3) setLock(false); // WINNER
                    changed = true; 
                }
            }
        }
    } 
    else if (strcmp(action, "finale_ready") == 0) {
        const char* kid = doc["kid"];
        bool isReady = doc["ready"];
        if (kid) {
            if (strcmp(kid, "sam") == 0) samReady = isReady;
            if (strcmp(kid, "kris") == 0) krisReady = isReady;
            if (strcmp(kid, "jacob") == 0) jacobReady = isReady;
            
            bool newArmed = (samReady && krisReady && jacobReady);
            if (newArmed != finaleArmed) {
                finaleArmed = newArmed;
                // Maybe auto-trigger lights?
            }
            changed = true;
        }
    }
    else if (strcmp(action, "unlock") == 0) {
        setLock(false);
        changed = true;
    }
    else if (strcmp(action, "lock") == 0) {
        setLock(true);
        changed = true;
    }
    else if (strcmp(action, "reset") == 0) {
        locked = true;
        samProgress = 0; krisProgress = 0; jacobProgress = 0;
        samReady = false; krisReady = false; jacobReady = false;
        finaleArmed = false;
        currentPattern = MODE_GAME;
        setLock(true);
        changed = true;
    }
    else if (strcmp(action, "pattern") == 0) {
        const char* mode = doc["mode"];
        if (mode) {
            if (strcmp(mode, "game") == 0) currentPattern = MODE_GAME;
            else if (strcmp(mode, "cylon") == 0) currentPattern = MODE_CYLON;
            else if (strcmp(mode, "meteor") == 0) currentPattern = MODE_METEOR;
            else if (strcmp(mode, "breathing") == 0) currentPattern = MODE_BREATHING;
            else if (strcmp(mode, "sparkle") == 0) currentPattern = MODE_SPARKLE;
            else if (strcmp(mode, "fire") == 0) currentPattern = MODE_FIRE;
            else if (strcmp(mode, "rainbow") == 0) currentPattern = MODE_RAINBOW;
            else if (strcmp(mode, "christmas") == 0) currentPattern = MODE_XMAS;
            else if (strcmp(mode, "off") == 0) currentPattern = MODE_OFF;
            changed = true;
        }
    }
    else if (strcmp(action, "anim") == 0) {
        const char* type = doc["type"];
        const char* state = doc["state"];
        if (type && strcmp(type, "speaking") == 0) {
            if (state && strcmp(state, "on") == 0) {
                currentPattern = MODE_SPEAKING;
            } else {
                currentPattern = MODE_GAME;
            }
            changed = true;
        }
    }
    

    else if (strcmp(action, "stop") == 0) {
        stopAudio();
        Serial.println("Audio stopped via MQTT");
    }
    else if (strcmp(action, "play") == 0) {
        const char* file = doc["file"];
        if (file) {
             scheduledStartTime = 0; // Immediate play
             playFileByName(file);
        }
    }
    else if (strcmp(action, "play_intro") == 0) {
        // Set character color based on box number
        if (strstr(client_id, "Box1") != NULL) {
            // Sam - Orange
            charColorR = 255; charColorG = 180; charColorB = 0;
        } else if (strstr(client_id, "Box2") != NULL) {
            // Kristine - Cyan
            charColorR = 0; charColorG = 255; charColorB = 255;
        } else if (strstr(client_id, "Box3") != NULL) {
            // Jacob - Orange-Red
            charColorR = 255; charColorG = 60; charColorB = 0;
        }

        // Schedule audio start based on 'delay' parameter (default 2000ms if missing)
        uint32_t syncDelay = doc["delay"] | 2000;
        
        // NO BUFFERING NEEDED for SPIFFS - Start immediately when sync time hits
        // We will just wait for the time.
        uint32_t startTime = millis() + syncDelay;

        // Start LED boot sequence
        introStartTime = startTime;
        currentPattern = MODE_INTRO_BOOT;
        Serial.println("Starting INTRO_BOOT LED pattern (synced with audio)");

        if (isConductor) {
            // CONDUCTOR: Send ESP-NOW broadcast with scheduled start time
            Serial.print("CONDUCTOR: Scheduling sync start at millis=");
            Serial.println(startTime);

            SyncMessage msg;
            msg.command = 1; // START_INTRO
            msg.startAtMs = startTime;

            esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&msg, sizeof(msg));
            if (result == ESP_OK) {
                Serial.println("ESP-NOW broadcast sent!");
            } else {
                Serial.println("ESP-NOW broadcast FAILED!");
            }
        } else {
            Serial.print("NON-CONDUCTOR: Scheduling start at millis=");
            Serial.println(startTime);
        }

        // ALL boxes schedule themselves via MQTT (ESP-NOW is backup/refinement)
        scheduledStartTime = startTime;
        waitingForScheduledStart = true; // Wait for sync time to start playing
        
        // NO Pre-connect needed for SPIFFS
        Serial.println("play_intro received: Scheduling SPIFFS playback.");
    }
    else if (strcmp(action, "intro_speaking") == 0) {
        // Control speaking/listening state during intro
        const char* state = doc["state"];
        if (state && strcmp(state, "on") == 0) {
            currentPattern = MODE_INTRO_SPEAKING;
            Serial.println("INTRO: Speaking ON");
        } else {
            currentPattern = MODE_INTRO_LISTENING;
            Serial.println("INTRO: Speaking OFF (listening)");
        }
    }

    if (changed) publishStatus();
}

// Non-blocking reconnect variables
unsigned long lastReconnectAttempt = 0;

void reconnect() {
    // If not connected, try to connect every 5s, but DO NOT BLOCK
    if (millis() - lastReconnectAttempt < 5000) return;
    lastReconnectAttempt = millis();

    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(client_id)) {
        Serial.println("connected");
        client.subscribe(box_topic_cmd);
        client.subscribe("lockbox/all/cmd"); // Broadcast topic for synchronized commands
        publishStatus();
    } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
    }
}

void updateLEDs();

// HEALTH CHECK
void runHealthCheck() {
    Serial.println("Running Health Check...");
    strip.clear();
    strip.show();

    // 1. RSSI Check (Signal Strength)
    long rssi = WiFi.RSSI();
    Serial.print("RSSI: "); Serial.println(rssi);
    bool wifiGood = (rssi > -75); // Warning if worse than -75dBm

    // 2. Battery Check (Placeholder for A0)
    // int val = analogRead(A0);
    // float voltage = val * (5.0 / 1023.0) * DIVIDER_RATIO;
    // bool batGood = (voltage > 3.6); 
    bool batGood = true; // Assume good for now

    if (wifiGood && batGood) {
        // SUCCESS: Green Pulse
        Serial.println("Health: PASS");
        for(int k=0; k<3; k++) { // 3 pulses
            for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, 0, 100, 0); // Dim Green
            strip.show();
            delay(300);
            strip.clear();
            strip.show();
            delay(200);
        }
    } else {
        // FAIL: Red Pulse
        Serial.println("Health: FAIL");
        for(int k=0; k<5; k++) { // 5 pulses
            for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, 100, 0, 0); // Dim Red
            strip.show();
            delay(500);
            strip.clear();
            strip.show();
            delay(200);
        }
    }
    strip.clear();
    strip.show();
}

void setup() {
    Serial.begin(115200);
    delay(100);
    
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW); // Start OFF (Solenoid retracted/ready)
    pinMode(AMP_SD_PIN, OUTPUT);
    digitalWrite(AMP_SD_PIN, LOW); // Start in Shutdown (Mute)
    setLock(true); // Logical lock state
    
    strip.begin();
    // POWER OPTIMIZATION: Hard cap at ~20%
    #define MAX_BRIGHTNESS 50 
    strip.setBrightness(MAX_BRIGHTNESS);
    strip.show();

    // START WIFI
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); // CRITICAL: Fixes Audio crackling/latency
    WiFi.begin(ssid, password);
    Serial.println("WiFi started...");

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

    udp.begin(UDP_PORT);
     Serial.print("UDP Listening on port "); Serial.println(UDP_PORT);

    // --- OTA SETUP ---
    ArduinoOTA.setHostname(client_id);
    ArduinoOTA.setPassword("admin");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_FS
            type = "filesystem";
        }
        // NOTE: if updating FS this would be the place to unmount FS using SPIFFS.end()
        Serial.println("Start updating " + type);
        
        // PAUSE EVERYTHING FOR STABILITY
        isUpdating = true;
        
        // Clear LEDs to save power
        strip.clear();
        strip.show();
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
        isUpdating = false;
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();
    Serial.println("OTA Ready");

    // --- ESP-NOW SETUP for Low-Latency Sync ---
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW Init Failed!");
    } else {
        Serial.println("ESP-NOW Initialized");
        
        // Register receive callback
        esp_now_register_recv_cb(onEspNowReceive);
        esp_now_register_send_cb(onEspNowSent);
        
        // Add broadcast peer
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, broadcastAddress, 6);
        peerInfo.channel = 0;  // Use current channel
        peerInfo.encrypt = false;
        
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.println("Failed to add broadcast peer");
        } else {
            Serial.println("Broadcast peer added");
        }
        
        // Box 1 (Sam) is the conductor - it sends the sync signal
        if (strstr(client_id, "box_1") != NULL) {
            isConductor = true;
            Serial.println("*** This box is the CONDUCTOR ***");
        }
    }

    // --- AUDIO SETUP ---
    audioLogger = &Serial;
    
    // Mount SPIFFS
    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    Serial.println("SPIFFS Mounted");

    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    out->SetGain(1.0); // MAX VOLUME: 100% gain + Loudness-Normalized Source
    out->SetOutputModeMono(true);

    // V1 ENGINEERING CHECK
    runHealthCheck();
}

void loop() {
    ArduinoOTA.handle();
    
    // CRITICAL: If updating, yield CPU to OTA and SKIP everything else
    if (isUpdating) return;

    // Maintain WiFi
    if (WiFi.status() != WL_CONNECTED) {
        // Maybe try to reconnect or just wait? ESP usually auto-reconnects if configured.
        // But if we want to confirm:
        static unsigned long lastWiFiCheck = 0;
        if (millis() - lastWiFiCheck > 5000) {
            lastWiFiCheck = millis();
            Serial.println("WiFi not connected...");
            // WiFi.reconnect(); // loop() will handle it
        }
    } else {
        // WiFi is good, check MQTT
        if (!client.connected()) {
            reconnect();
        }
        client.loop();
        
        // HEARTBEAT DEBUG
        static unsigned long lastHeartbeat = 0;
        if (millis() - lastHeartbeat > 5000) {
            lastHeartbeat = millis();
            publishStatus();
        }
    }
    
    // --- SCHEDULED SYNC PLAYBACK TRIGGER ---
    if (waitingForScheduledStart) {
        // Wait until the scheduled start time
        if (millis() >= scheduledStartTime) {
            waitingForScheduledStart = false; // Reset flag
            Serial.print("SYNC: Starting audio at millis=");
            Serial.println(millis());

            // Play generic "intro.mp3" from SPIFFS (specific file uploaded to each box)
            playFileByName("/intro.mp3");
        }
    }
    
    // SOLENOID MONITOR
    if (lockPulseStart > 0 && (millis() - lockPulseStart > LOCK_PULSE_MS)) {
        digitalWrite(RELAY_PIN, LOW); // End Pulse
        lockPulseStart = 0;
    }
    
    audioLoop();
    
    updateLEDs(); 
    
    // UDP LISTENER
    int packetSize = udp.parsePacket();
    if (packetSize) {
        char packetBuffer[255];
        int len = udp.read(packetBuffer, 255);
        if (len > 0) packetBuffer[len] = 0;
        
        Serial.print("UDP Rx: "); Serial.println(packetBuffer);
        
        // Simple Protocol: "START", "OFF", "XMAS" based on first char or string
        // To avoid self-loop, maybe check IP? UDP broadcast is received by sender too usually.
        // For now, let's just parse.
        String msg = String(packetBuffer);
        if (msg.startsWith("SYNC:")) {
            String modeStr = msg.substring(5);
            if (modeStr == "OFF") currentPattern = MODE_OFF;
            else if (modeStr == "XMAS") currentPattern = MODE_XMAS;
            else if (modeStr == "GAME") currentPattern = MODE_GAME;
            publishStatus();
        }
    }

    // POWER OPTIMIZATION: Yield removed for Audio Performance
    // delay(20); 
}

void broadcastUDP(String msg) {
    // Easier: 255.255.255.255
    IPAddress broadcastIp(255, 255, 255, 255);
    
    udp.beginPacket(broadcastIp, UDP_PORT);
    udp.print(msg);
    udp.endPacket();
    Serial.print("UDP Tx: "); Serial.println(msg);
}

// ----------------------------------------------------------------------
// LED PATTERNS (Same as before)
// ----------------------------------------------------------------------

void updateGameLEDs() {
    // Limit update rate to prevent WiFi starvation (30 FPS)
    if (millis() - lastUpdate < 33) return;
    lastUpdate = millis();

    strip.clear();
    
    if (!locked) {
        // VICTORY RAINBOW
        for(int i=0; i<NUM_LEDS; i++) {
           strip.setPixelColor(i, strip.ColorHSV((millis() / 5 + i * 65536 / NUM_LEDS) % 65536));
        }
    } else if (finaleArmed) {
        // PULSING RED ALERT
        int b = (millis() / 5) % 255;
        if (b > 127) b = 255 - b; 
        uint32_t c = strip.Color(b*2, 0, 0); 
        for(int i=0; i<NUM_LEDS; i++) strip.setPixelColor(i, c);
    } else {
        // LEDs 0-2: Sam
        for(int i=0; i<3; i++) {
            if(i < samProgress) strip.setPixelColor(i, 255, 180, 0);
            else strip.setPixelColor(i, 20, 10, 0); 
        }
        // LEDs 4-6: Kristine
        for(int i=4; i<7; i++) {
            if(i-4 < krisProgress) strip.setPixelColor(i, 0, 255, 255);
            else strip.setPixelColor(i, 0, 20, 20);
        }
        // LEDs 8-10: Jacob
        for(int i=8; i<11; i++) {
            if(i-8 < jacobProgress) strip.setPixelColor(i, 255, 60, 0);
            else strip.setPixelColor(i, 20, 5, 0);
        }
    }
    strip.show();
}

void updateCylon() {
    if (millis() - lastUpdate < 10) return;
    lastUpdate = millis();
    static int pos = 0;
    static int dir = 1;
    static int eyeSize = 10;
    strip.clear();
    for(int j=0; j<eyeSize; j++) {
        if(pos+j < NUM_LEDS) strip.setPixelColor(pos+j, 255, 0, 0);
    }
    strip.show();
    pos += dir;
    if (pos >= NUM_LEDS - eyeSize - 2 || pos <= 0) dir = -dir;
}

void updateMeteor() {
    if (millis() - lastUpdate < 30) return;
    lastUpdate = millis();
    byte meteorSize = 10;
    byte decay = 64;
    for(int j=0; j<NUM_LEDS; j++) {
        if(random(10)>5) {
            uint32_t old = strip.getPixelColor(j);
            uint8_t r = (old >> 16) & 0xFF;
            uint8_t g = (old >> 8) & 0xFF;
            uint8_t b = old & 0xFF;
            r = (r<=10)?0:(r-(r*decay/256));
            g = (g<=10)?0:(g-(g*decay/256));
            b = (b<=10)?0:(b-(b*decay/256));
            strip.setPixelColor(j, r, g, b);
        }
    }
    static int i = 0;
    for(int j = 0; j < meteorSize; j++) {
        if( ( i-j < NUM_LEDS) && (i-j>=0) ) {
            strip.setPixelColor(i-j, 0, 0, 255); 
        }
    }
    strip.show();
    i++;
    if(i >= NUM_LEDS + NUM_LEDS) i = 0;
}

void updateBreathing() {
    if (millis() - lastUpdate < 10) return;
    lastUpdate = millis();
    static int i = 0;
    float intensity = (exp(sin(i/2000.0*PI)) - 0.36787944)*108.0;
    strip.fill(strip.Color(0, 0, 255*intensity/255));
    strip.show();
    i = (i + 10) % 65535;
}

void updateSparkle() {
    if (millis() - lastUpdate < 50) return;
    lastUpdate = millis();
    strip.clear(); // Clear previous
    int p = random(NUM_LEDS);
    // POWER OPTIMIZATION: Warm White instead of Max White
    strip.setPixelColor(p, 200, 200, 150);
    strip.show();
}

void updateFire() {
    if (millis() - lastUpdate < 30) return;
    lastUpdate = millis();
    #define FIRE_WIDTH 60 
    static byte fireHeat[FIRE_WIDTH];
    for(int i=0; i<FIRE_WIDTH; i++) {
        fireHeat[i] = max(0, (int)(fireHeat[i] - random(0, ((55*10)/FIRE_WIDTH) + 2)));
    }
    for(int k=FIRE_WIDTH-1; k>=2; k--) {
        fireHeat[k] = (fireHeat[k-1] + fireHeat[k-2] + fireHeat[k-2])/3;
    }
    if(random(255) < 120) {
        int y = random(7);
        fireHeat[y] = min(255, (int)(fireHeat[y] + random(160, 255)));
    }
    for(int j=0; j<NUM_LEDS; j++) {
        int heatIdx = j % FIRE_WIDTH; 
        byte t = fireHeat[heatIdx];
        byte t192 = round((t/255.0)*191);
        byte heatramp = t192 & 0x3F; heatramp <<= 2;
        if(t192 > 0x80) strip.setPixelColor(j, 255, 255, heatramp);
        else if(t192 > 0x40) strip.setPixelColor(j, 255, heatramp, 0);
        else strip.setPixelColor(j, heatramp, 0, 0);
    }
    strip.show();
}

void updateRainbow() {
    if (millis() - lastUpdate < 10) return;
    lastUpdate = millis();
    static long firstPixelHue = 0;
    for(int i=0; i<NUM_LEDS; i++) { 
        int pixelHue = firstPixelHue + (i * 65536L / NUM_LEDS);
        strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    firstPixelHue += 256;
}

void updateXmas() {
    if (millis() - lastUpdate < 500) return;
    lastUpdate = millis();
    static bool toggle = false;
    toggle = !toggle;
    strip.clear(); // POWER OPTIMIZATION: Sparse fill
    for(int i=0; i<NUM_LEDS; i+=2) { // Every other pixel
        if ((i % 4 == 0) == toggle) {
            strip.setPixelColor(i, 255, 0, 0); 
        } else {
            strip.setPixelColor(i, 0, 255, 0); 
        }
    }
    strip.show();
}

void updateLEDs() {
    static PatternMode lastPattern = MODE_GAME;

    // POWER OPTIMIZATION: Amp Control
    if (currentPattern == MODE_OFF) {
        digitalWrite(AMP_SD_PIN, LOW); // Shutdown Amp
    } else {
        digitalWrite(AMP_SD_PIN, HIGH); // Wake Amp
    }

    // POWER OPTIMIZATION: LED Isolation
    if (currentPattern == MODE_OFF) {
        if (lastPattern != MODE_OFF) {
            strip.clear();
            strip.show(); // One last clear to turn off pixels
        }
        // Do NOT call strip.show() repeatedly in OFF mode to save idle power
        lastPattern = currentPattern;
        return; 
    }

    lastPattern = currentPattern;

    switch (currentPattern) {
        case MODE_GAME: updateGameLEDs(); break;
        case MODE_CYLON: updateCylon(); break;
        case MODE_METEOR: updateMeteor(); break;
        case MODE_BREATHING: updateBreathing(); break;
        case MODE_SPARKLE: updateSparkle(); break;
        case MODE_FIRE: updateFire(); break;
        case MODE_RAINBOW: updateRainbow(); break;
        case MODE_XMAS: updateXmas(); break;
        case MODE_SPEAKING: updateSpeaking(); break;
        case MODE_INTRO_BOOT: updateIntroBoot(); break;
        case MODE_INTRO_SPEAKING: updateIntroSpeaking(); break;
        case MODE_INTRO_LISTENING: updateIntroListening(); break;
        // MODE_OFF handled above
    }
}

void updateSpeaking() {
    // Random flicker to simulate voice
    if (millis() - lastUpdate < 50) return;
    lastUpdate = millis();

    // Pick a random LED or range
    strip.clear();

    // Voice Amplitude Simulation
    int intensity = random(50, 255);
    int numActive = random(5, NUM_LEDS/4);

    for(int i=0; i<numActive; i++) {
        int idx = random(NUM_LEDS);
        // Golden/White voice color
        strip.setPixelColor(idx, strip.Color(intensity, intensity*0.8, intensity*0.2));
    }
    strip.show();
}

// ----------------------------------------------------------------------
// INTRO LIGHT SHOW PATTERNS
// ----------------------------------------------------------------------

void updateIntroBoot() {
    // Red cylon sweep that accelerates over 14 seconds, then transitions to listening
    if (millis() - lastUpdate < 10) return;
    lastUpdate = millis();

    // If introStartTime is in the future (waiting for sync), show static dim red
    if (millis() < introStartTime) {
        strip.clear();
        // Show a dim center glow while waiting
        for (int i = CENTER_LED - 5; i <= CENTER_LED + 5; i++) {
            if (i >= 0 && i < NUM_LEDS) {
                strip.setPixelColor(i, strip.Color(30, 0, 0)); // Dim red
            }
        }
        strip.show();
        return;
    }

    unsigned long elapsed = millis() - introStartTime;

    // At 15 seconds (when dialogue starts), flash to character color and transition to listening
    if (elapsed >= 15000) {
        // Brief flash at center
        strip.clear();
        for (int i = CENTER_LED - 20; i <= CENTER_LED + 20; i++) {
            if (i >= 0 && i < NUM_LEDS) {
                strip.setPixelColor(i, strip.Color(charColorR, charColorG, charColorB));
            }
        }
        strip.show();
        delay(100);

        // Transition to listening mode
        currentPattern = MODE_INTRO_LISTENING;
        return;
    }

    // Calculate cylon speed - starts slow, accelerates
    // Speed factor: starts at 1.0, reaches 4.0 at 15 seconds
    float progress = (float)elapsed / 15000.0;
    float speedFactor = 1.0 + (progress * 3.0);

    // Position calculation with acceleration
    // Base cycle time is 2000ms, accelerates to 500ms
    float cycleTime = 2000.0 / speedFactor;
    float cycleProgress = fmod((float)elapsed, cycleTime) / cycleTime;

    // Ping-pong motion
    int pos;
    if (cycleProgress < 0.5) {
        pos = (int)(cycleProgress * 2.0 * (NUM_LEDS - 1));
    } else {
        pos = (int)((1.0 - (cycleProgress - 0.5) * 2.0) * (NUM_LEDS - 1));
    }

    // Draw cylon eye with trail
    strip.clear();
    int eyeSize = 5;
    int trailLength = 15;

    for (int i = -trailLength; i <= eyeSize; i++) {
        int idx = pos + i;
        if (idx >= 0 && idx < NUM_LEDS) {
            float brightness;
            if (i >= 0 && i <= eyeSize) {
                brightness = 1.0; // Full brightness for eye
            } else {
                brightness = 1.0 - ((float)(-i) / trailLength); // Fade trail
            }
            int r = (int)(255 * brightness);
            strip.setPixelColor(idx, strip.Color(r, 0, 0));
        }
    }
    strip.show();
}

void updateIntroSpeaking() {
    // Center-out sparkle effect in character color
    if (millis() - lastUpdate < 50) return; // 20 FPS
    lastUpdate = millis();

    strip.clear();

    // Number of active sparkles - more than listening, fewer than full strip
    int numSparkles = random(15, 35);

    for (int i = 0; i < numSparkles; i++) {
        // Bias sparkles toward center using gaussian-like distribution
        int offset = random(-80, 81); // Range of ±80 from center
        // Weight toward center: smaller offsets more likely
        if (random(100) < 50) {
            offset = offset / 2; // 50% chance to halve the offset
        }

        int idx = CENTER_LED + offset;
        if (idx >= 0 && idx < NUM_LEDS) {
            // Slight brightness variation for sparkle effect
            float sparkle = 0.7 + (random(30) / 100.0); // 0.7 to 1.0
            int r = (int)(charColorR * sparkle);
            int g = (int)(charColorG * sparkle);
            int b = (int)(charColorB * sparkle);

            // Occasional white highlight
            if (random(100) < 15) {
                r = min(255, r + 50);
                g = min(255, g + 50);
                b = min(255, b + 50);
            }

            strip.setPixelColor(idx, strip.Color(r, g, b));
        }
    }
    strip.show();
}

void updateIntroListening() {
    // Static dim glow in character color - center region only
    // Low update rate since it's mostly static
    if (millis() - lastUpdate < 100) return; // 10 FPS
    lastUpdate = millis();

    strip.clear();

    // Light center region (LEDs 80-126, ~47 LEDs)
    int startLED = 80;
    int endLED = 126;

    // Dim brightness - about 30% of character color
    int r = charColorR * 0.3;
    int g = charColorG * 0.3;
    int b = charColorB * 0.3;

    for (int i = startLED; i <= endLED; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
}

// ----------------------------------------------------------------------
// AUDIO IMPLEMENTATION
// ----------------------------------------------------------------------
void stopAudio() {
  if (mp3) { mp3->stop(); delete mp3; mp3 = NULL; }
  if (id3) { delete id3; id3 = NULL; } // Delete ID3 source
  if (file) { file->close(); delete file; file = NULL; }
  // Keep output open or delete if needed
}

// playAudio removed

void audioLoop() {
  if (mp3 && mp3->isRunning()) {
    if (!mp3->loop()) {
      mp3->stop();
      digitalWrite(AMP_SD_PIN, LOW); // Mute
    }
  }
}

void playFileByName(const char* filename) {
  stopAudio();

  Serial.printf("Playing SPIFFS file: %s\n", filename);

  file = new AudioFileSourceSPIFFS(filename);
  
  if (!file) {
      Serial.println("Failed to open file!");
      return;
  }
  
  id3 = new AudioFileSourceID3(file);
  mp3 = new AudioGeneratorMP3();
  mp3->begin(id3, out);
  
  digitalWrite(AMP_SD_PIN, HIGH); // Unmute
  Serial.println("Playback started.");
}
    


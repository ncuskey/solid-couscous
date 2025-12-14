#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

// ----------------------------------------------------------------------
// CONFIGURATION
// ----------------------------------------------------------------------
const char* ssid = "Mickey Mouse Club House";
const char* password = "sunnyjax1787";

const char* mqtt_server = "167.172.211.213";
const int mqtt_port = 1883;

// CHANGE THIS FOR EACH BOX: lockbox_1, lockbox_2, etc. (Match CONFIG in web app)
// For simplicity we are using "lockbox/1/..." as topic base.
// Ideally make this dynamic or user-configurable.
const char* box_topic_cmd = "lockbox/2/cmd";
const char* box_topic_status = "lockbox/2/status";
const char* client_id = "box_2_holiday";

// ----------------------------------------------------------------------
// HARDWARE PINS
// ----------------------------------------------------------------------
#define RELAY_PIN 5  
#define LED_PIN   4  
#define NUM_LEDS  207 

// ----------------------------------------------------------------------
// GLOBAL OBJECTS
// ----------------------------------------------------------------------
WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ----------------------------------------------------------------------
// GAME STATE
// ----------------------------------------------------------------------
bool locked = true;
int samProgress = 0;   // 0-3
int krisProgress = 0;  // 0-3
int jacobProgress = 0; // 0-3

// Finale Synchronization State
bool samReady = false;
bool krisReady = false;
bool jacobReady = false;
bool finaleArmed = false;

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
    MODE_OFF = 99
};

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

// ----------------------------------------------------------------------
// LOGIC
// ----------------------------------------------------------------------
void setLock(bool state) {
    locked = state;
    digitalWrite(RELAY_PIN, locked ? LOW : HIGH); 
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
                if (strcmp(kid, "sam") == 0 && samProgress < 3) { samProgress++; changed = true; }
                if (strcmp(kid, "kris") == 0 && krisProgress < 3) { krisProgress++; changed = true; }
                if (strcmp(kid, "jacob") == 0 && jacobProgress < 3) { jacobProgress++; changed = true; }
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
        publishStatus();
    } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
    }
}

void updateLEDs();

void setup() {
    Serial.begin(115200);
    delay(100);
    
    pinMode(RELAY_PIN, OUTPUT);
    setLock(true);
    
    strip.begin();
    strip.setBrightness(50);
    strip.show();

    // Non-blocking WiFi check in loop, but here we can wait a bit or just proceed.
    // For lights to work immediately, we should ideally not block here either, but standard practice is to wait for WiFi.
    // Let's optimize: Just start WiFi, and let loop handle connection status.
    WiFi.begin(ssid, password);
    Serial.println("WiFi started...");

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);

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
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
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
}

void loop() {
    ArduinoOTA.handle();

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
    }
    
    updateLEDs(); 
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
    int p = random(NUM_LEDS);
    strip.setPixelColor(p, 255, 255, 255);
    strip.show();
    delay(10); 
    strip.setPixelColor(p, 0, 0, 0); 
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
    for(int i=0; i<NUM_LEDS; i++) {
        if ((i % 2 == 0) == toggle) {
            strip.setPixelColor(i, 255, 0, 0); 
        } else {
            strip.setPixelColor(i, 0, 255, 0); 
        }
    }
    strip.show();
}

void updateLEDs() {
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
        case MODE_OFF: strip.clear(); strip.show(); break;
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

# Christmas Lockboxes 🎄🔐

A distributed, synchronized Narrative Escape Room experience powered by ESP32, MQTT, and Web Technologies.

## 🚀 Features (V2 Release - Christmas 2024)

### 🎙️ Radio Play System
- **Synchronized Audio**: Three boxes play character-specific audio tracks in perfect sync
- **MQTT Coordination**: All boxes receive `play_intro` command and start playback after 2-second sync delay
- **ESP-NOW Backup**: Conductor (Box 1) can broadcast low-latency sync signals
- **Character Mapping**:
  - Box 1 = Sam (Conductor)
  - Box 2 = Kristine
  - Box 3 = Jacob

### 🎭 Radio Play Controller
- **`radio_play.html`**: Web interface for triggering synchronized playback
  - "Play on Boxes" - Sends MQTT command to all physical boxes
  - "Play All (Browser)" - Preview all tracks in browser
  - Individual track playback for testing
  - Live dialogue display with timestamps

### 🔌 Power Management
- **Smart Standby**: WiFi sleep disabled for audio streaming stability
- **Amp Shutdown**: Pin D14 controls MAX98357A enable/disable
- **LED Brightness Cap**: Hard limit at 50 (20%) to prevent brownouts

### 🏥 Health & Reliability
- **Self-Test**: Startup check for WiFi RSSI (> -75dBm)
- **Visual Feedback**:
    - 🟢🟢🟢 Green Pulse = Ready
    - 🔴🔴🔴 Red Pulse = Weak Signal / Error
- **OTA Updates**: All boxes support over-the-air firmware updates (password: `admin`)

## Hardware Config
- **Lockbox 1 (Sam)**: 192.168.7.199 (Conductor)
- **Lockbox 2 (Kristine)**: 192.168.7.201
- **Lockbox 3 (Jacob)**: 192.168.7.202
- **Server**: `167.172.211.213` (MQTT + Web)
- **MQTT Port:** 1883 (Device), 9001 (WebSockets)
- **Web App:** `http://167.172.211.213/xmasLockboxes/`
- **Radio Play:** `http://167.172.211.213/xmasLockboxes/radio_play.html`
- **Admin Panel:** `http://167.172.211.213/xmasLockboxes/admin.html`

## Setup for Multiple Boxes

### Firmware Configuration
Edit `ChristmasLockbox/ChristmasLockbox.ino` lines 34-36:
```cpp
// Box 1 = Sam (CONDUCTOR), Box 2 = Kristine, Box 3 = Jacob
const char* box_topic_cmd = "lockbox/1/cmd";    // Change 1 to 2 or 3
const char* box_topic_status = "lockbox/1/status";
const char* client_id = "box_1_v2";             // Change to box_2_v2 or box_3_v2
```

### OTA Firmware Update
```bash
# Compile
/opt/homebrew/bin/arduino-cli compile --fqbn esp32:esp32:esp32 \
  --libraries /path/to/xmasLockboxes/libraries \
  --output-dir /path/to/xmasLockboxes/ChristmasLockbox/build \
  /path/to/xmasLockboxes/ChristmasLockbox/ChristmasLockbox.ino

# Upload (replace IP with target box)
/opt/homebrew/bin/arduino-cli upload -p 192.168.7.199 \
  --fqbn esp32:esp32:esp32 \
  -i /path/to/xmasLockboxes/ChristmasLockbox/build/ChristmasLockbox.ino.bin \
  --upload-field password=admin
```

## Deployment
Deploy web updates to server:
```bash
scp -i ssh -r www/* root@167.172.211.213:/var/www/xmasLockboxes/
```

## Audio Files
Radio play audio files (in `www/audio/`):
- `Intro_Sam.mp3` - Box 1 character audio (Sam's lines only)
- `Intro_Kristine.mp3` - Box 2 character audio (Kristine's lines only)
- `Intro_Jacob.mp3` - Box 3 character audio (Jacob's lines only)

Timeline:
- 0-10s: Power-up sounds
- 10-15s: Settling/ambient
- 15s+: Dialogue begins (see `show_sequence.json` for exact timestamps)

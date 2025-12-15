# Christmas Lockboxes 🎄🔐

A distributed, synchronized Narrative Escape Room experience powered by ESP8266, MQTT, and Web Technologies.

## 🚀 Features (V1 Engineering Release)

### 🔌 Advanced Power Management
- **Smart Standby**: Modem Sleep & Loop Delays (<0.6W idle).
- **Hard Interrupts**: Software "Kill Switch" for LEDs and Amp Shutdown (Pin D5) in standby.
- **Deep Sleep**: Wi-Fi radio sleeps between beacons.

### 🎭 Director Mode
- **Central Control**: `simulator.html` acts as the "Director", sending sync commands to all boxes.
- **Lip Sync**: Real-time animation triggers (`speaking:on/off`) synchronized with audio.
- **UDP Conductor**: Zero-latency broadcast for "Start Show" commands.

### 🏥 Health & Reliability
- **Self-Test**: Startup check for WiFi RSSI (> -75dBm).
- **Visual Feedback**:
    - 🟢🟢🟢 Green Pulse = Ready
    - 🔴🔴🔴 Red Pulse = Weak Signal / Error
- of **Visual Ready Cue**.

## 📱 Mobile Command Center
- **Web App**: Hosted `admin.html` optimized for iPhone (Add to Home Screen).
- **Control**: Trigger Radio Play, Unlock/Reset boxes, and monitor status.

## Hardware Config
- **Lockbox 1 (Sam)**: 192.168.7.195 (Conductor)
- **Lockbox 2 (Kristine)**: 192.168.7.194
- **Lockbox 3 (Jacob)**: *Pending Build*
- **Server**: `167.172.211.213` (MQTT + Web)
- **MQTT Port:** 1883 (Device), 9001 (WebSockets)
- **Web App:** `http://167.172.211.213/`
- **Admin Panel:** `http://167.172.211.213/admin.html`

## Setup for Multiple Boxes
1.  **Firmware (`ChristmasLockbox.ino`):**
    - Open the file in Arduino IDE.
    - Locate the configuration section at the top.
    - Update `box_topic_cmd`, `box_topic_status`, and `client_id` for the specific box (e.g., Change `1` to `2`).
    - Flash to the ESP8266.

2.  **Web Config (`www/config.js`):**
    - Edit `www/config.js` (and upload to server).
    - Add the new box to the `BOXES` array:
      ```javascript
      BOXES: [
          { name: "Lockbox 1", topic: "lockbox/1" },
          { name: "Lockbox 2", topic: "lockbox/2" }
      ]
      ```

## Deployment
To deploy web updates:
```bash
scp -r www/* root@167.172.211.213:/var/www/html/
```

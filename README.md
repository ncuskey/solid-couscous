# Christmas Lockboxes Project

## Overview
Interactive puzzle lockboxes controlled via WiFi/MQTT.

## Infrastructure
- **Server IP:** `167.172.211.213`
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

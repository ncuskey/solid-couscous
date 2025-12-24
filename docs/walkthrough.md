# Solenoid Migration Walkthrough

## Changes Implemented
The firmware (`ChristmasLockbox.ino`) has been updated to support the new solenoid latch hardware. Be sure to flash the new firmware before connecting the solenoid to prevent damage (the old firmware would hold the solenoid ON indefinitely).

### Firmware Logic
*   **Mode**: Non-blocking Pulse.
*   **Pin**: GPIO 33 (Relay Pin).
*   **Action**: 
    1.  `unlockBox()` -> writes `HIGH` to GPIO 33.
    2.  `loop()` checks timer.
    3.  After **300ms**, `loop()` writes `LOW` to GPIO 33.
*   **Safety**: The code prevents the pin from staying HIGH even if WiFi/MQTT disconnects during the pulse.

## Verification Steps (Perform in Order)

### 1. Dry Run (No Solenoid)
1.  **Disconnect** the solenoid from the MOSFET.
2.  **Flash** the updated firmware to the ESP32.
3.  **Boot** the system.
4.  **Unlock** the box via the web interface or MQTT command.
5.  **Measure** voltage at GPIO 33 (or the MOSFET Gate):
    *   Expected: A brief spike to 3.3V (approx 0.3s) then back to 0V.
    *   It should **NOT** stay high.

### 2. Voltage Check (Boost Converter)
1.  Verify the **5V Input** is stable.
2.  Measure the **XL6009 Output**.
3.  Adjust the trim pot until it reads exactly **12.0V**.

### 3. Live Test
1.  Power **OFF** everything.
2.  Connect the **Solenoid** to the 12V rail and MOSFET Drain.
3.  Ensure the **Flyback Diode** is in place (Stripe to +12V).
4.  Power **ON**.
5.  Trigger an unlock.
    *   **Result**: A distinct "Click" of the solenoid retracting, followed immediately by release.
    *   **Check**: Ensure the solenoid does not get hot. If it starts warming up rapidly, disconnect immediately—the MOSFET might be stuck on or the pin logic inverted.

## Wiring Reference
Refer to [solenoid_migration.md](solenoid_migration.md) for the complete wiring diagram if needed.

## Troubleshooting Notes
*   **Brownout / Reset on Click**: 
    *   *Cause*: Inrush current starving the ESP32. 
    *   *Fix*: Ensure 5V source is >2A (Battery/Wall Plug, not Laptop USB).
    *   *Fix*: Ensure **1000µF capacitor** is on the 12V rail to buffer the pulse.
*   **12V Rail Short (0V Output)**:
    *   *Check*: Verify Capacitor polarity (Stripe to Ground).
    *   *Check*: Verify Flyback Diode orientation (Stripe to +12V).

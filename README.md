# HearSync — Directional Sound & Priority Alert Wearable

HearSync is an assistive wearable system designed to provide real-time directional awareness and priority haptic feedback for environmental sounds (emergency sirens, vehicle horns, and ambient sounds) for the deaf and hard-of-hearing.

---

## Architecture Overview

```
                      [ Environmental Sound ]
                                 │
                    ┌────────────▼────────────┐
                    │  MEMS I2S Microphones   │
                    │  (Direction & Acoustic) │
                    └────────────┬────────────┘
                                 │ Audio Samples (I2S DMA)
                    ┌────────────▼────────────┐
                    │      ESP32 DevKit       │
                    │  Directional TDOA Engine│
                    │  & Priority AI Analysis │
                    └──────┬────────────┬─────┘
                           │            │
             ┌─────────────▼─────┐    ┌─▼──────────────────┐
             │ 16x2 I2C Display  │    │ Directional Haptic │
             │ Real-Time Status  │    │ Actuators (Motors) │
             └───────────────────┘    │ Left / Center / R  │
                                      └────────────────────┘
```

---

## Hardware Configuration (Wokwi Simulation & Prototype)

- **Microcontroller**: ESP32 DevKit V1
- **Display**: 16x2 LCD with PCF8574 I2C adapter (Address: `0x27`)
  - `SDA` ➔ GPIO 21
  - `SCL` ➔ GPIO 22
  - `VCC` ➔ 3.3V
  - `GND` ➔ GND
- **Directional Haptic Indicators (Motors / LEDs)**:
  - **Left Actuator (Blue)**: GPIO 18
  - **Center Actuator (Yellow)**: GPIO 19
  - **Right Actuator (Red)**: GPIO 23
- **Simulation Control**:
  - **Direction Compass Knob**: GPIO 34 (ADC1)
    - **West / Left** (`< 1700`): Blue Left LED
    - **North / South** (`1700 - 2400`): Yellow Center LED (direct 12 o'clock alignment)
    - **East / Right** (`> 2400`): Red Right LED
  - **Siren Trigger Pushbutton**: GPIO 14 (Simulates emergency audio event)

---

## Priority Feedback Language

| Sound Event | Priority Level | Visual Output (16x2 LCD) | Haptic Feedback Pattern |
|---|---|---|---|
| **Ambient / Quiet** | **LOW** | `DIR: <DIRECTION>`<br>`[<--] SOUND READY` | Selected direction indicator steady ON |
| **Vehicle Horn** | **MEDIUM** | `! VEHICLE HORN !`<br>`FROM: <DIRECTION>` | **2 distinct pulses** on target motor |
| **Emergency Siren** | **HIGH** | `!! SIREN ALERT !!`<br>`FROM: <DIRECTION>` | **3 rapid urgent pulses** on target motor |

---

## Getting Started

### 1. Requirements
- [VS Code](https://code.visualstudio.com/)
- [PlatformIO IDE Extension](https://platformio.org/)
- [Wokwi Simulator Extension](https://wokwi.com/vscode)

### 2. Build Firmware
```bash
pio run
```

### 3. Run Simulation
Open `diagram.json` in VS Code and press `F1` ➔ **`Wokwi: Start Simulator`**.

---

## Project Structure

```text
Hear_Sync/
│
├── src/
│   └── main.cpp         # Core ESP32 firmware (Direction engine & priority haptics)
│
├── diagram.json         # Complete Wokwi simulation circuit layout
├── platformio.ini       # PlatformIO configuration & dependencies
├── wokwi.toml           # Wokwi VS Code extension configuration
├── libraries.txt        # Wokwi cloud simulation library list
└── README.md            # Project documentation
```

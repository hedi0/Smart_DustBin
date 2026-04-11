# 🗑️ Smart Dustbin System

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue?style=for-the-badge)
![License](https://img.shields.io/badge/license-Apache--2.0-green?style=for-the-badge)
![Platform](https://img.shields.io/badge/platform-Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![Language](https://img.shields.io/badge/language-C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)

![Stars](https://img.shields.io/github/stars/hedi0/Smart_DustBin?style=for-the-badge)
![Forks](https://img.shields.io/github/forks/hedi0/Smart_DustBin?style=for-the-badge)

### A contactless, battery-powered smart bin — automatically opens when you approach, monitors its own battery, and sleeps to save power when idle.
 
![Preview](preview_example.png)
 
</div>

---

## 📋 Table of Contents


1. [How It Works](#-how-it-works)
2. [Features](#-features)
3. [What You Need](#-what-you-need)
4. [Wiring It Up](#-wiring-it-up)
5. [Software Setup](#-software-setup)
6. [Configuration](#-configuration)
7. [Usage](#-usage)
8. [Power & Battery Life](#-power--battery-life)
9. [Project Structure](#-project-structure)
10. [Troubleshooting](#-troubleshooting)
11. [Future Ideas](#-future-ideas)
12. [Safety Notes](#-safety-notes)

---

## 💡 How It Works

The system runs a simple, repeating loop:

```
Sleep  →  Wake  →  Check battery  →  Measure distance  →  Open lid  →  Wait  →  Close lid  →  Sleep
```

1. The Arduino stays in **deep sleep** to save battery.
2. A **watchdog timer** wakes it up every second.
3. It fires the **ultrasonic sensor** to check if anything is within 15 cm.
4. If something is detected → the **servo opens the lid**.
5. After 3 seconds with nothing detected → the lid **closes automatically**.
6. It goes back to sleep.

> If battery voltage drops below **3.0V**, the system enters permanent sleep to protect the cell.

---

## ✨ Features

### 🎯 Core

- **Hands-free lid** — opens when you're within 15 cm, no touching needed
- **Auto-close** — lid closes after 3 seconds of no detection
- **Deep sleep mode** — draws only ~1–5 mA when idle
- **Watchdog wake-up** — wakes the MCU on a timer without user interaction
- **Interrupt-driven ultrasonic** — echo measured via hardware interrupt (accurate & non-blocking)

### 🔋 Battery & Safety
- **Live voltage monitoring** — reads Li-ion voltage using the internal 1.1V ADC reference
- **Low battery LED warning** — LED blinks when voltage drops below 3.3V
- **Critical cutoff at 3.0V** — permanently sleeps to prevent cell damage
- **Peripheral shutdown** — disables SPI, I²C, UART when not in use

---

## 🛠️ What You Need

### Hardware

| # | Component | Notes |
|---|-----------|-------|
| 1 | Arduino Uno or Nano (ATmega328P) | Main controller |
| 2 | HC-SR04 Ultrasonic Sensor | Detects hand proximity |
| 3 | SG90 or MG90S Servo Motor | Opens/closes the lid |
| 4 | 18650 Li-ion Battery (**protected**) | Power source |
| 5 | TP4056 Charging Module (**with protection**) | Safe charging |
| 6 | 470µF Electrolytic Capacitor | Stabilizes servo power |
| 7 | LED + 220Ω resistor | Low battery indicator (optional) |
| 8 | Jumper wires + breadboard | — |

> ⚠️ **Important:** Use a **protected** 18650 cell and a TP4056 **with** the protection circuit. Using unprotected components is a fire hazard..

### Software

| Tool | Link |
|------|------|
| Arduino IDE | https://www.arduino.cc/en/software |
| `Servo.h` | Built into Arduino IDE |
| `avr/sleep.h` | Built into Arduino IDE |
| `avr/power.h` | Built into Arduino IDE |
| `avr/wdt.h` | Built into Arduino IDE |

No ext libraries to install — everything is included with the Arduino IDE.

---

## 🔌 Wiring It Up

```
Arduino Pin    →    Component
──────────────────────────────────────────
D2             →    HC-SR04   TRIG
D3             →    HC-SR04   ECHO
D9  (PWM)      →    Servo     SIGNAL (orange wire)
D13            →    LED (+)   → 220Ω resistor → GND
5V             →    HC-SR04   VCC
5V             →    Servo     VCC  ──┬── 470µF cap ──┐
GND            →    All       GND  ──┘               │
                                                    GND
```

**Quick checklist before powering on:**
- [ ] All GND pins share a common ground
- [ ] 470µF capacitor is across the servo's VCC and GND
- [ ] TP4056 protection circuit is present (not bare module)
- [ ] Servo signal goes goes to D9 (PWM-capable pin)

---

## ⚙️ Software Setup

### Step 1 — Install Arduino IDE
Download from: https://www.arduino.cc/en/software

### Step — Open the sketch
```
File → Open → codes/smart_dustbin.ino
```

### Step 3 — Select your board and port
```
Tools → Board   → Arduino Uno  (or Nano w/ ATmega328P)
Tools → Port    → COM3  (or your port — check Device Manager on Windows)
```

### Step 4 — Upload
Click the **→ Upload** button. Done.

---

## 🔧 Configuration

All settings are at the top of `smart_dustbin.ino`. Edit these to match your build:
```cpp
// ── Detection ─────────────────────────────────────────
#define DETECT_DISTANCE_CM   15      // Open lid if object closer than this (cm)
#define LID_CLOSE_DELAY_MS   3000    // How long to keep lid open (milliseconds)

// ── Servo Angles ──────────────────────────────────────
#define SERVO_OPEN_DEG       90      // Angle when lid is OPEN
#define SERVO_CLOSED_DEG     0       // Angle when lid is CLOSED

// ── Battery Thresholds ────────────────────────────────
#define BATT_LOW_V           3.3f    // LED warning turns on below this voltage
#define BATT_CRITICAL_V      3.0f    // System sleeps permanently below this
```

---

## 🚀 Usage

Once powered on, the system runs fully automatically. Here's the complete flow:

```
┌─────────────────────────────────────────────┐
│              POWER ON / BOOT                │
│  Initialize pins, servo, watchdog timer     │
└────────────────────┬────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────┐
│              DEEP SLEEP                     │
│  Current draw: ~1–5 mA                      │
└────────────────────┬────────────────────────┘
                     │  ← Watchdog wakes MCU every ~1s
                     ▼
┌─────────────────────────────────────────────┐
│           CHECK BATTERY VOLTAGE             │
│  Below 3.0V? → Permanent sleep (LED solid)  │
│  Below 3.3V? → LED blinks (low warning)     │
└────────────────────┬────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────┐
│            MEASURE DISTANCE                 │
│   Fire ultrasonic pulse → read echo time    │
│   Nothing within 15 cm? → Back to sleep     │
└────────────────────┬────────────────────────┘
                     │  ← Object detected ≤ 15 cm
                     ▼
┌─────────────────────────────────────────────┐
│            OPEN LID                         │
│  Servo rotates to SERVO_OPEN_DEG            │
│  Wait LID_CLOSE_DELAY_MS (default 3s)       │
│  Servo rotates to SERVO_CLOSED_DEG          │
└────────────────────┬────────────────────────┘
                     │
                     └──────────────► Back to Deep Sleep
```

### LED Status Guide

| LED State | Meaning |
|-----------|---------|
| Off | Normal — system sleeping or operating fine |
| Slow blink | Battery low — below 3.3V, charge soon |
| Solid on | Critical — battery below 3.0V, system halted |

---

## 🔋 Power & Battery Life

### Current Draw by Mode

| Mode | Current |
|------|---------|
| Servo moving | 200 – 500 mA (peak) |
| MCU awake + 


















# 🗑️ Smart Dustbin System

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license/Smart_DustBin-green)
![Stars](https://img.shields.io/github/stars/hedi0/Smart_DustBin?style=social)
![Forks](https://img.shields.io/github/forks/hedi0/Smart_DustBin?style=social)

An intelligent waste management system designed to optimize waste collection and promote cleanliness through sensor-based monitoring and automation.

![example-preview-image](/preview_example.png)


------------------------------------------------------------------------

# 📋 Table of Contents

-   [Overview](#-overview)
-   [Features](#-features)
-   [Hardware Requirements](#-hardware-requirements)
-   [Professional Wiring Schematic](#-professional-wiring-schematic)
-   [Software Requirements](#-software-requirements)
-   [Installation Guide](#-installation-guide)
-   [Fingerprint Enrollment](#-fingerprint-enrollment)
-   [System Operation](#-system-operation)
-   [Configuration](#-configuration)
-   [Security Considerations](#-security-considerations)
-   [Troubleshooting](#-troubleshooting)
-   [Future Improvements](#-future-improvements)

------------------------------------------------------------------------

## ✨ Features

### 🎯 Core Functionality

-   Automatic Lid Opening (15 cm detection)
-   Servo-Controlled Lid Mechanism
-   Auto-Close Timer (3 seconds)
-   Interrupt-Based Ultrasonic Reading
-   Watchdog Wake-Up System
-   Deep Sleep Mode for Power Saving

### 🔋 Battery & Safety Features

-   Li-ion Battery Monitoring
-   Low Battery LED Warning
-   Critical 3.0V Cutoff Protection
-   Internal 1.1V Reference Voltage Reading
-   Peripheral Shutdown for Reduced Power Consumption

------------------------------------------------------------------------

## 🛠️ Hardware Requirements

-   Arduino Uno / Nano (ATmega328P)
-   HC-SR04 Ultrasonic Sensor
-   SG90 / MG90 Servo Motor
-   18650 Li-ion Battery (Protected)
-   TP4056 Charging Module (with protection)
-   470µF Capacitor
-   LED (Optional)
-   Jumper Wires
-   Breadboard

⚠️ Use a protected Li-ion battery and add a 470µF capacitor across the
servo power lines.

------------------------------------------------------------------------

## 🔌 Pin Connections

-   D2 → Ultrasonic Trig\
-   D3 → Ultrasonic Echo\
-   D9 → Servo Signal\
-   D13 → LED\
-   GND → Common Ground

------------------------------------------------------------------------

## ⚙️ Software Setup

Install Arduino IDE from: https://www.arduino.cc/en/software

Required Libraries: - Servo - avr/sleep.h - avr/power.h - avr/wdt.h

------------------------------------------------------------------------

## 🔧 Configuration

Detection: - Distance: 15 cm - Close delay: 3000 ms

Battery Thresholds: - Low: 3.3V - Critical: 3.0V

------------------------------------------------------------------------

## 🚀 Usage

1.  System wakes from deep sleep.
2.  Measures distance.
3.  Opens lid if object detected.
4.  Clooses after delay.
5.  Returns to sleep.
6.  Enters permanent sleep if battery \< 3.0V.

------------------------------------------------------------------------

## 🔋 Power Optimization

Estimated Current: - Servo: 200--500mA - Active: 20--40mA - Sleep:
\~1--5mA

Advanced: - Add MOSFET for servo power - Power ultrasonic via
transistor - Disable brown-out detection - Use 8MHz internal clock

------------------------------------------------------------------------

## 📁 Project Structure

Smart-Dustbin/
├── smart_dustbin.ino
├── LICENSE 
└── README.md

------------------------------------------------------------------------

## 🔄 Future Enhancements

-   Solar charging
-   ESP32 IoT version
-   OLED battery display
-   Odor sensor integration

------------------------------------------------------------------------

## ⚠️ Safety Notes

-   Never discharge Li-ion below 3.0V
-   Use TP4056 with protection
-   Ensure common ground
-   Add capacitor for servo stability









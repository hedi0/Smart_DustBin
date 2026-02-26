/*
 * Smart Dustbin - Li-ion Optimized Version
 * Safe for single 3.7V Li-ion cell
 */

#include <Servo.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

// Pin definitions
const int trigPin = 2;
const int echoPin = 3;
const int servoPin = 9;
const int powerLED = 13;

// Configuration
const int OPEN_ANGLE = 90;
const int CLOSE_ANGLE = 0;
const int DETECTION_DISTANCE = 15;
const unsigned long CLOSE_DELAY = 3000;

const float LOW_BATTERY_THRESHOLD = 3.3;
const float CRITICAL_BATTERY_THRESHOLD = 3.0;

Servo lidServo;

volatile unsigned long pulseStart = 0;
volatile unsigned long pulseEnd = 0;
volatile bool pulseReceived = false;
volatile bool echoTimeout = false;

bool isOpen = false;
unsigned long lastOpenTime = 0;

float batteryVoltage = 0.0;
bool lowBattery = false;

// -------------------- SETUP --------------------

void setup() {

  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);

  pinMode(echoPin, INPUT_PULLUP);
  pinMode(powerLED, OUTPUT);
  digitalWrite(powerLED, LOW);

  attachInterrupt(digitalPinToInterrupt(echoPin), echoInterrupt, CHANGE);

  checkBattery();
  closeLid();

  setupWatchdog();
}

// -------------------- MAIN LOOP --------------------

void loop() {

  // Critical battery protection
  if (batteryVoltage < CRITICAL_BATTERY_THRESHOLD) {
    while (1) {
      enterDeepSleep();
    }
  }

  checkBattery();
  if (!lowBattery) {

    long distance = measureDistance();

    if (distance > 0 && distance <= DETECTION_DISTANCE) {

      lidServo.attach(servoPin);
      delay(50);

      openLid();
      lastOpenTime = millis();

      while (millis() - lastOpenTime < CLOSE_DELAY) {
        delay(100);
      }

      closeLid();
      detachServo();
    }
  } 
  else {
    blinkLowBatteryWarning();
  }

  enterDeepSleep();
}






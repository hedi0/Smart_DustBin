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

// -------------------- ULTRASONIC --------------------

long measureDistance() {

  pulseReceived = false;
  echoTimeout = false;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long startTime = micros();

  while (!pulseReceived) {
    if (micros() - startTime > 30000) {
      echoTimeout = true;
      break;
    }
  }

  if (echoTimeout || !pulseReceived) return -1;

  unsigned long duration = pulseEnd - pulseStart;
  long distance = duration * 0.0343 / 2;

  if (distance < 2 || distance > 400) return -1;

  return distance;
}

void echoInterrupt() {
  if (digitalRead(echoPin) == HIGH) {
    pulseStart = micros();
  } else {
    pulseEnd = micros();
    pulseReceived = true;
  }
}

// -------------------- SERVO --------------------

void openLid() {
  if (!isOpen) {
    lidServo.write(OPEN_ANGLE);
    delay(500);
    isOpen = true;
  }
}

void closeLid() {
  if (isOpen) {
    lidServo.write(CLOSE_ANGLE);
    delay(500);
    isOpen = false;
  }
}

void detachServo() {
  if (lidServo.attached()) {
    lidServo.detach();
    pinMode(servoPin, INPUT);
  }
}

// -------------------- BATTERY (Accurate Li-ion Reading) --------------------

long readVcc() {
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC));

  long result = ADCL;
  result |= ADCH << 8;
  result = 1125300L / result;
  return result; // mV
}

void checkBattery() {
  power_adc_enable();

  long vcc = readVcc();
  batteryVoltage = vcc / 1000.0;

  lowBattery = (batteryVoltage < LOW_BATTERY_THRESHOLD);

  power_adc_disable();
}

// -------------------- LOW BATTERY WARNING --------------------

void blinkLowBatteryWarning() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(powerLED, HIGH);
    delay(100);
    digitalWrite(powerLED, LOW);
    delay(100);
  }
}

// -------------------- WATCHDOG + SLEEP --------------------

void setupWatchdog() {
  MCUSR &= ~(1 << WDRF);
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  WDTCSR = (1 << WDP3) | (1 << WDP0) | (1 << WDIE); // 8s
  WDTCSR &= ~(1 << WDE);
}

void enterDeepSleep() {

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  ADCSRA &= ~(1 << ADEN);
  power_all_disable();


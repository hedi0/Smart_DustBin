
/*
 * Smart Dustbin with Battery Optimization
 * Auto-opens lid when object detected within 15cm
 * Closes lid after 3 seconds
 * Uses deep sleep to save battery power
 */

#include <Servo.h>
#include <avr/sleep.h>      // For sleep modes
#include <avr/power.h>      // For disabling peripherals

// Pin definitions
const int trigPin = 2;      // Ultrasonic trigger pin (supports interrupt)
const int echoPin = 3;      // Ultrasonic echo pin (supports interrupt)
const int servoPin = 9;     // Servo control pin (PWM capable)
const int batteryPin = A0;  // Battery voltage monitoring
const int powerLED = 13;    // Status indicator LED

// Configuration constants
const int OPEN_ANGLE = 90;           // Servo angle when lid is open
const int CLOSE_ANGLE = 0;           // Servo angle when lid is closed
const int DETECTION_DISTANCE = 15;   // Distance threshold in cm
const unsigned long CLOSE_DELAY = 3000;      // Time lid stays open (ms)
const unsigned long WAKE_INTERVAL = 1000;    // Normal wake interval
const int BATTERY_CHECK_INTERVAL = 10;       // Battery check frequency
const float LOW_BATTERY_THRESHOLD = 3.3;     // Low battery voltage

// Global variables
Servo lidServo;                         // Servo object
volatile bool motionDetected = false;   // Interrupt flag for motion
bool isOpen = false;                    // Current lid state
unsigned long lastOpenTime = 0;         // Last time lid was opened
int wakeCount = 0;                      // Count of wake cycles
int lidOpenCount = 0;                   // Total lid openings
float batteryVoltage = 0.0;             // Measured battery voltage
bool lowBattery = false;                // Low battery flag

// Ultrasonic sensor interrupt variables
volatile unsigned long pulseStart = 0;  // Echo pulse start time
volatile unsigned long pulseEnd = 0;    // Echo pulse end time
volatile bool pulseReceived = false;    // Pulse received flag
volatile bool echoTimeout = false;      // Timeout flag

// Setup function - runs once at startup
void setup() {
  // Disable unused peripherals to save power
  power_adc_disable();
  power_spi_disable();
  power_twi_disable();
  power_timer1_disable();  // Keep Timer0 for delay()
  
  // Configure pins for low power operation
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  pinMode(echoPin, INPUT);
  digitalWrite(echoPin, LOW);
  pinMode(powerLED, OUTPUT);
  digitalWrite(powerLED, LOW);
  pinMode(servoPin, OUTPUT);
  
  // Set up interrupt for echo pin to measure pulse duration
  attachInterrupt(digitalPinToInterrupt(echoPin), echoInterrupt, CHANGE);
  
  // Initial system checks
  checkBattery();    // Measure battery voltage
  closeLid();        // Ensure lid is closed
  setupWatchdog();   // Configure watchdog timer for wake-up
}

// Main loop - runs repeatedly after setup
void loop() {
  // Brief LED flash to indicate wake cycle
  digitalWrite(powerLED, HIGH);
  delay(10);
  digitalWrite(powerLED, LOW);
  
  // Check battery voltage periodically
  if (wakeCount % BATTERY_CHECK_INTERVAL == 0) {
    checkBattery();
  }
  
  // Only operate if battery level is sufficient
  if (!lowBattery) {
    // Measure distance to nearest object
    long distance = measureDistance();
    
    // Check if object is within detection range
    if (distance > 0 && distance <= DETECTION_DISTANCE) {
      // Power up servo and attach it
      lidServo.attach(servoPin);
      delay(50);  // Wait for servo to initialize
      
      // Open the lid
      openLid();
      lastOpenTime = millis();
      lidOpenCount++;
      
      // Monitor while lid is open
      while (isOpen) {
        // Check if it's time to close the lid
        if (millis() - lastOpenTime > CLOSE_DELAY) {
          closeLid();
          detachServo();      // Detach to save power
          break;
        }
        
        // Check if object is still present
        distance = measureDistance();
        if (distance > 0 && distance <= DETECTION_DISTANCE) {
          // Reset timer if object is still there
          lastOpenTime = millis();
        }
        
        delay(100);  // Small delay between checks
      }
    } else {
      // No object detected, power down servo
      detachServo();
    }
  } else {
    // Low battery mode - warn user with LED blinks
    blinkLowBatteryWarning();
  }
  
  // Increment wake counter and enter deep sleep
  wakeCount++;
  enterDeepSleep();
}

// Measure distance using ultrasonic sensor
long measureDistance() {
  // Enable ADC for measurement
  power_adc_enable();
  
  // Reset measurement flags
  pulseReceived = false;
  echoTimeout = false;
  
  // Send 10µs trigger pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Wait for echo with timeout (30ms = ~5m range)
  unsigned long startTime = micros();
  while (!pulseReceived && !echoTimeout) {
    if (micros() - startTime > 30000) {
      echoTimeout = true;
      break;
    }
  }
  
  // Disable ADC to save power
  power_adc_disable();
  
  // Return -1 if measurement failed
  if (echoTimeout || !pulseReceived) {
    return -1;
  }
  
  // Calculate distance from pulse duration
  unsigned long duration = pulseEnd - pulseStart;
  long distance = duration * 0.0343 / 2;  // Speed of sound = 343 m/s
  
  // Filter unrealistic values (2-400 cm range)
  if (distance < 2 || distance > 400) {
    return -1;
  }
  
  return distance;
}

// Interrupt handler for echo pin state changes
void echoInterrupt() {
  if (digitalRead(echoPin) == HIGH) {
    // Echo pulse started - record start time
    pulseStart = micros();
  } else {
    // Echo pulse ended - record end time
    pulseEnd = micros();
    pulseReceived = true;
  }
}

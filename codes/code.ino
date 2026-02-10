
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

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


// Open the dustbin lid
void openLid() {
  if (!isOpen) {
    lidServo.write(OPEN_ANGLE);  // Move servo to open position
    delay(500);                  // Wait for servo to reach position
    isOpen = true;               // Update lid state
  }
}

// Close the dustbin lid
void closeLid() {
  if (isOpen) {
    lidServo.write(CLOSE_ANGLE); // Move servo to closed position
    delay(500);                  // Wait for servo to reach position
    isOpen = false;              // Update lid state
  }
}

// Detach servo to save power
void detachServo() {
  if (lidServo.attached()) {
    lidServo.detach();           // Detach servo object
    // Set pin to input to prevent power leakage
    pinMode(servoPin, INPUT);
    digitalWrite(servoPin, LOW);
  }
}

// Measure battery voltage
void checkBattery() {
  // Enable ADC for measurement
  power_adc_enable();
  ADCSRA |= (1 << ADEN);  // Turn on ADC
  
  // Read analog value from voltage divider
  int sensorValue = analogRead(batteryPin);
  
  // Convert to voltage (assumes 5V reference and 2:1 divider)
  batteryVoltage = sensorValue * (5.0 / 1023.0) * 2.0;
  
  // Check if voltage is below threshold
  lowBattery = (batteryVoltage < LOW_BATTERY_THRESHOLD);
  
  // Disable ADC to save power
  ADCSRA &= ~(1 << ADEN);
  power_adc_disable();
}

// Blink LED to indicate low battery
void blinkLowBatteryWarning() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(powerLED, HIGH);
    delay(100);
    digitalWrite(powerLED, LOW);
    delay(100);
  }
}

// Configure watchdog timer for automatic wake-up
void setupWatchdog() {
  MCUSR &= ~(1 << WDRF);        // Clear watchdog reset flag
  WDTCSR |= (1 << WDCE) | (1 << WDE);  // Enable configuration change
  // Set watchdog timeout to 8 seconds and enable interrupt
  WDTCSR = (1 << WDP3) | (0 << WDP2) | (0 << WDP1) | (1 << WDP0) | (1 << WDIE);
  WDTCSR &= ~(1 << WDE);        // Disable watchdog reset, keep interrupt
}

// Enter deep sleep mode to save power
void enterDeepSleep() {
  // Configure sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Lowest power mode
  sleep_enable();                       // Enable sleep
  
  // Disable peripherals for maximum power saving
  ADCSRA &= ~(1 << ADEN);               // Turn off ADC
  power_all_disable();                  // Disable all peripherals
  
  // Enable interrupts and enter sleep
  sei();                                // Enable interrupts
  sleep_cpu();                          // Enter sleep mode
  
  // Code resumes here after wake-up
  sleep_disable();                      // Disable sleep mode
  power_all_enable();                   // Re-enable peripherals
}

// Watchdog timer interrupt service routine
ISR(WDT_vect) {
  // Empty - just wakes up the microcontroller
  // No action needed, just triggers wake-up from sleep
}

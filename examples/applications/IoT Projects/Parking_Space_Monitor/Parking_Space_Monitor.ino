// -----------------------------------------------------------------------------
// Project: Smart Parking Space Monitor System
// File: Smart_Parking_Monitor.ino
// Description: Uses an ultrasonic sensor to detect if a parking slot is occupied
//              and reports its status (vacant/parked) and the measured distance
//              to the Arduino IoT Cloud.
// -----------------------------------------------------------------------------

// Include necessary libraries for Arduino IoT Cloud and ultrasonic sensor.
#include "thingProperties.h"   // Auto-generated by Arduino IoT Cloud
#include <Ultrasonic.h>        // For ultrasonic distance sensor (e.g., HC-SR04)

// -----------------------------------------------------------------------------
// Pin Definitions
// -----------------------------------------------------------------------------
#define ULTRASONIC_TRIG_PIN D6 // Digital pin for Ultrasonic sensor Trigger
#define ULTRASONIC_ECHO_PIN D7 // Digital pin for Ultrasonic sensor Echo

// -----------------------------------------------------------------------------
// Sensor Object
// -----------------------------------------------------------------------------
Ultrasonic ultrasonic(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN); // Ultrasonic sensor object

// -----------------------------------------------------------------------------
// Constants and Thresholds
// -----------------------------------------------------------------------------
// Adjust this threshold based on your parking slot setup and sensor placement.
// If the measured distance is LESS than or EQUAL to this value, the slot is considered PARKED.
// If the measured distance is GREATER than this value, the slot is considered VACANT.
const int PARKED_DISTANCE_THRESHOLD_CM = 100; // Example: 100 cm (1 meter)

const long SENSOR_READ_INTERVAL = 2000; // Read sensor every 2 seconds

// -----------------------------------------------------------------------------
// Global Variables (for internal use, not directly linked to Cloud Properties)
// -----------------------------------------------------------------------------
unsigned long lastSensorReadTime = 0; // To control sensor reading frequency

// -----------------------------------------------------------------------------
// Setup Function: Runs once when the board starts
// -----------------------------------------------------------------------------
void setup() {
  // Initialize Serial communication for debugging
  Serial.begin(9600);
  while (!Serial); // Wait for Serial Monitor to open

  // Initialize Arduino IoT Cloud properties
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  // Set debug message level for more detailed output
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // Initialize Cloud properties to default values
  slotStatus = false; // Default to parked or unknown
  distanceCm = 0;

  Serial.println("Setup complete for Smart Parking Space Monitor.");
}

// -----------------------------------------------------------------------------
// Loop Function: Runs repeatedly
// -----------------------------------------------------------------------------
void loop() {
  // Update Arduino IoT Cloud state (sends/receives data)
  ArduinoCloud.update();

  // Read ultrasonic sensor periodically
  if (millis() - lastSensorReadTime > SENSOR_READ_INTERVAL) {
    readParkingStatus();
    lastSensorReadTime = millis();
  }
}

// -----------------------------------------------------------------------------
// Function to read ultrasonic sensor and update Cloud properties
// -----------------------------------------------------------------------------
void readParkingStatus() {
  long measuredDistanceCm = ultrasonic.read(); // Read distance in centimeters

  // Update Cloud property with the raw measured distance
  distanceCm = measuredDistanceCm;

  // Determine the slot status based on the threshold
  // If the measured distance is greater than the threshold, the slot is VACANT.
  // Otherwise, it's considered PARKED.
  if (measuredDistanceCm > PARKED_DISTANCE_THRESHOLD_CM) {
    if (!slotStatus) { // Only print if status changes
      Serial.println("Parking Slot: VACANT");
    }
    slotStatus = true; // Set to true for VACANT
  } else {
    if (slotStatus) { // Only print if status changes
      Serial.println("Parking Slot: PARKED");
    }
    slotStatus = false; // Set to false for PARKED
  }

  Serial.print("Distance: "); Serial.print(measuredDistanceCm); Serial.print(" cm | Status: ");
  Serial.println(slotStatus ? "VACANT" : "PARKED");
}

// -----------------------------------------------------------------------------
// Arduino IoT Cloud Callbacks (read-only properties, so callbacks are for monitoring)
// -----------------------------------------------------------------------------

/*
  Callback function for when the slotStatus property changes in the Cloud.
  This is a read-only property, so this callback is primarily for debugging/monitoring.
*/
void onSlotStatusChange() {
  Serial.print("Cloud attempted to change Slot Status to: ");
  Serial.println(slotStatus ? "VACANT" : "PARKED");
}

/*
  Callback function for when the distanceCm property changes in the Cloud.
  This is a read-only property, so this callback is primarily for debugging/monitoring.
*/
void onDistanceCmChange() {
  Serial.print("Cloud attempted to change Distance (cm) to: ");
  Serial.println(distanceCm);
}

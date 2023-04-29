/* Arduino Hot Plate Cycler + Temp recorder v1
 * Code based on code from:
 * https://github.com/adafruit/MAX6675-library/blob/master/examples/serialthermocouple/serialthermocouple.ino
 * https://protosupplies.com/product/max6675-thermocouple-temperature-module/
 * https://be189.github.io/lessons/10/control_of_arduino_with_python.html
 */

//===============================================================================
//  Imports
//===============================================================================
#include <math.h>
/* Arduino library for interfacing with MAX6675 thermocouple amplifier
 * https://github.com/adafruit/MAX6675-library
 * Must be installed before first use:
 * Sketch > Include Library > Managed Libraries > "MAX6675 library" by Adafruit
 */
#include "max6675.h"

//===============================================================================
//  Defining magic numbers
//===============================================================================
// Rate at which information is transferred in a communication channel
// (Probably) needs to match the value of the variable in the Python code  
int const BAUDRATE = 9600;
// Delay before data recording starts, to give MAX6675 chips time to stabilize
int const DELAY = 1000;

//===============================================================================
//  Preliminaries
//===============================================================================
// Handshake
const int HANDSHAKE = 0;
const int TEMP_REQUEST = 1;
const int ON_REQUEST = 2;
const int STREAM = 3;
const int READ_DAQ_DELAY = 4;

// Two data acquisition (DAQ) modes, on-request and stream
// Initially, only send data upon request
int daqMode = ON_REQUEST;
// Default time between data acquisition is 100 ms
// This will be overwritten following handshake
int daqDelay = 100;
// String to store input of DAQ delay
String daqDelayStr;
// Keep track of last data acquistion for delays
unsigned long timeOfLastDAQ = 0;

// Pins
/* MCU is microcontroller
 * https://protosupplies.com/product/max6675-thermocouple-temperature-module/
 * GND = Connect to system ground. This ground needs to be in common with the MCU.
 * UCC = Connect to 3.0 to 5.5V. Typically connected to the power of the MCU.
 * SCK = Serial Clock Input. Connect to any digital pin on MCU.
 * SO = Chip Select. Connect to any digital pin on MCU.
 * CS = Serial Data Ouput. Connect to any digital pin on MCU.
 */

// Initializing pins
// PWM
int pwm = 13;
// First MAX6675
int thermoDO1 = 4;
int thermoCS1 = 5;
int thermoCLK1 = 6;
// Second MAX6675
int thermoDO2 = 8;
int thermoCS2 = 9;
int thermoCLK2 = 10;

// Creating instances of MAX6675 control
// Note to self: I *think* this means instances
// in the sense of allocating memory for a new object
MAX6675 thermocouple1(thermoCLK1, thermoCS1, thermoDO1);
MAX6675 thermocouple2(thermoCLK2, thermoCS2, thermoDO2);

//===============================================================================
//  Functions
//===============================================================================

//// unsigned long printTemp(MAX6675 thermocouple) {
unsigned long printTemp() {
  // Reads the time and temp for a SINGLE thermocouple
  // Writes it to where it can be processed in Python

  // Read value from thermocouple
  float value1 = thermocouple1.readCelsius();
  float value2 = thermocouple2.readCelsius();
  
  // Get the time point
  unsigned long timeMilliseconds = millis();

  // Write the result
  if (Serial.availableForWrite()) {
    String outstr = String(String(timeMilliseconds, DEC) + "," + String(value1, DEC) + "," + String(value2, DEC));
    Serial.println(outstr);
  }

  // Return time of acquisition
  return timeMilliseconds;
}

//===============================================================================
//  Initialization
//===============================================================================
void setup() {
  // Set serial commucation speed with computer
  Serial.begin(BAUDRATE);
  
  // HOT PLATE CYCLE
  // Initialize pin (should be 13) as an output
  pinMode(pwm, OUTPUT);
  
  // TEMP RECORDING
  // Wait for MAX6675 chips to stabilize
  delay(DELAY);
}

//===============================================================================
//  Main
//===============================================================================
void loop() {
  
  // If we're streaming
  if (daqMode == STREAM) {
    // TEMP RECORDING
    // Periodically checking temperature
    if (millis() - timeOfLastDAQ >= daqDelay) {
      timeOfLastDAQ = printTemp();
    }

//    // HOT PLATE CYCLE
//    // So we wait until data is streaming to cycle
//    // Turn off if it's been on too long
//    if (millis() - timeOfLastOn >= daqDelay) {
//      timeOfLastDAQ = printTemp();
//    }
//    
//    int timeOn = 100;
//    int timeOff = 1000 - timeOn;
//      
//    digitalWrite(13, HIGH);
//    delay(timeOn);
//    digitalWrite(13, LOW);
//    delay(timeOff);
  }
  
  // Check if data has been sent to Arduino and respond accordingly
  // If the Python code isn't running, this'll do nothing
  if (Serial.available() > 0) {
    // Read in request
    int inByte = Serial.read();
    
    // If data is requested, fetch it and write it, or handshake
    switch(inByte) {
      case TEMP_REQUEST:
        timeOfLastDAQ = printTemp();
        break;
      case ON_REQUEST:
        daqMode = ON_REQUEST;
        break;
      case STREAM:
        daqMode = STREAM;
        break;
      case READ_DAQ_DELAY:
        // Read in delay, knowing it is appended with an x
        daqDelayStr = Serial.readStringUntil('x');

        // Convert to int and store
        daqDelay = daqDelayStr.toInt();

        break;
      case HANDSHAKE:
        if (Serial.availableForWrite()) {
          Serial.println("Message received.");
        }
        break;
    }
  }
}

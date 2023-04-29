/* I'm just trying to get it to send the time normally, which it somehow struggles to do
 * This works perfectly while the other doesn't? HOW???
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
int const DELAY = 0; // 1000;

//===============================================================================
//  Preliminaries
//===============================================================================
// Handshake
const int HANDSHAKE = 0;
const int TIME_REQUEST = 1;
const int ON_REQUEST = 2;
const int STREAM = 3;
const int READ_DAQ_DELAY = 4;

// Two data acquisition (DAQ) modes, on-request and stream
// Initially, only send data upon request
int daqMode = ON_REQUEST;
// Default time between data acquisition is 100 ms
int daqDelay = 0; // 100; ((((((THIS WILL GET OVERWRITTEN BY THE PYTHON PROGRAM))))))
// String to store input of DAQ delay
String daqDelayStr;
// Keep track of last data acquistion for delays
unsigned long timeOfLastDAQ = 0;

//===============================================================================
//  Functions
//===============================================================================

//// unsigned long printTemp(MAX6675 thermocouple) {
unsigned long printTime() {
  // Get the time point
  unsigned long timeMilliseconds = millis();

  // Write the result
  if (Serial.availableForWrite()) {
    String outstr = String(timeMilliseconds, DEC);
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
}

//===============================================================================
//  Main
//===============================================================================
void loop() {
  
  // If we're streaming
  if (daqMode == STREAM) {
    // TIME RECORDING
    if (millis() - timeOfLastDAQ >= daqDelay) {
      //Serial.print(String(millis(), DEC));
      timeOfLastDAQ = printTime();
    }
  }
  
  // Check if data has been sent to Arduino and respond accordingly
  // If the Python code isn't running, this'll do nothing
  if (Serial.available() > 0) {
    // Read in request from Jupyter Notebook
    int inByte = Serial.read();
    
    // If data is requested, fetch it and write it, or handshake
    switch(inByte) {
      case TIME_REQUEST:
        timeOfLastDAQ = printTime();
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

// this example is public domain. enjoy! https://learn.adafruit.com/thermocouple/
// Code from https://computers.tutsplus.com/tutorials/how-to-read-temperatures-with-arduino--mac-53714
#include <math.h> //loads the more advanced math functions 
const int HANDSHAKE = 0;
const int TEMP_REQUEST = 1;
const int ON_REQUEST = 2;
const int STREAM = 3;
const int READ_DAQ_DELAY = 4;

int daqMode = ON_REQUEST;

unsigned long timeOfLastDAQ = 0;

int daqDelay = 100;

String daqDelayStr;

#include "max6675.h"

int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

unsigned long printTemp() {
  delay(1000);
  // Read value
  double value = thermocouple.readFahrenheit();

  // Get the time point
  unsigned long timeMilliseconds = millis();

  // Write the result
  if (Serial.availableForWrite()) {
    String outstr = String(String(timeMilliseconds, DEC) + "," + String(value, DEC));
    Serial.println(outstr);
  }

  // Return time of acquisition
  return timeMilliseconds;
}

void setup() {
  Serial.begin(9600);
  
  // wait for MAX chip to stabilize
  //delay(500);
}

void loop() {
  // If we're streaming
  if (daqMode == STREAM) {
    if (millis() - timeOfLastDAQ >= daqDelay) {
      timeOfLastDAQ = printTemp();
    }
  }

  // Check if data has been sent to Arduino and respond accordingly
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

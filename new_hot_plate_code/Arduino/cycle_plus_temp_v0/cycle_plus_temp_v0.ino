/*
  Arduino Hot Plate Cycler + Temp recorder v0
*/

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

// Make sure to install this library first!
// Sketch > Include Library > Managed Libraries > "MAX6675 library" by Adafruit
#include "max6675.h"

int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;

int thermoDO2 = 8;
int thermoCS2 = 9;
int thermoCLK2 = 10;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
MAX6675 thermocouple2(thermoCLK2, thermoCS2, thermoDO2);

unsigned long printTemp() {
  delay(1000);
  // Read value
  double value = thermocouple.readFahrenheit();
  double value2 = thermocouple2.readFahrenheit();

  // Get the time point
  unsigned long timeMilliseconds = millis();

  // Write the result
  if (Serial.availableForWrite()) {
    String outstr = String(String(timeMilliseconds, DEC) + "," + String(value, DEC)+ "," + String(value2, DEC));
    Serial.println(outstr);
  }

  // Return time of acquisition
  return timeMilliseconds;
}

void setup() {
  Serial.begin(9600);
  
  // HOT PLATE CYCLE
  // initialize pin 13 as an output.
  pinMode(13, OUTPUT);
  
  // TEMP RECORDING
  // wait for MAX chip to stabilize
  //delay(500);
}

void loop() {
  // HOT PLATE CYCLE
  int timeOn = 100;
  int timeOff = 1000 - timeOn;
    
  digitalWrite(13, HIGH);
  delay(timeOn);
  digitalWrite(13, LOW);
  delay(timeOff);

  // TEMP RECORDING
  printTemp();
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

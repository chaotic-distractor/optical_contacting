/* Arduino Hot Plate Cycler + Temp recorder v3
 * Based on code from:
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

// Handshake
const int HANDSHAKE = 0;
const int TEMP_REQUEST = 1;
const int ON_REQUEST = 2;
const int STREAM = 3;
const int READ_DAQ_DELAY = 4;
const int READ_HEAT_PARAMS = 5;

// Rate at which information is transferred in a communication channel
// (Probably) needs to match the value of the variable in the Python code  
int const BAUDRATE = 9600;
// Delay before data recording starts, to give MAX6675 chips time to stabilize
int const DELAY = 1000;

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

//===============================================================================
//  Structs
//===============================================================================

// Temperature reading
struct tempRead {
  float temp1;
  float temp2;
  unsigned long timeMilliseconds;
};

// Hot plate cycle
  struct cycle {
  int timeOn;
  int timeOff;
};

//===============================================================================
//  Preliminaries
//===============================================================================

// *declared up here because it cannot be declared in the switch statement

// TEMP RECORDING
// Two data acquisition (DAQ) modes, on-request and stream
// Initially, only send data upon request
int daqMode = ON_REQUEST;
// Default time between data acquisition is 100 ms
// This will be overwritten following handshake
int daqDelay = 100;
// String to store input of DAQ delay*
String daqDelayStr;
// Keep track of last data acquistion for delays
unsigned long timeOfLastDAQ = 0;

// HOT PLATE CYCLE
// Keep track if hot plate is on/off
bool plateOn = false;
// Keep track of how long since it last turned on or off
unsigned long timeOfLastSwitch = 0;
// String to store input of heating rate and maximum temperature*
String rateMaxStr;
// Intermittent variable for parsing the above*
int comma;
// Default heating rate and max temp
// This will be overwritten following handshake
float heatingRate = 0.0;
float maxTemp = 0.0;
// Current temperature of hot plate
struct tempRead currentRead;
// Current hot plate cycle rate
struct cycle currentCycle;

// Creating instances of MAX6675 control
// Note to self: I *think* this means instances
// in the sense of allocating memory for a new object
MAX6675 thermocouple1(thermoCLK1, thermoCS1, thermoDO1);
MAX6675 thermocouple2(thermoCLK2, thermoCS2, thermoDO2);

//===============================================================================
//  Functions
//===============================================================================

struct tempRead printTemp() {
  // Reads the time and temp for both thermocouples
  // Writes it to where it can be processed in Python
  struct tempRead reading;
  
  // Read value from thermocouple
  reading.temp1 = thermocouple1.readCelsius();
  reading.temp2 = thermocouple2.readCelsius();

  // Get the time point
  reading.timeMilliseconds = millis();

  // Write the result
  if (Serial.availableForWrite()) {
    String outstr = String(String(reading.timeMilliseconds, DEC) + "," +
      String(reading.temp1, DEC) + "," + String(reading.temp2, DEC) + "," +
      String(plateOn, DEC));
    Serial.println(outstr);
  }

  // Return temps and time of acquisition
  return reading;
}

struct cycle adjustRate(struct tempRead reading) {
  //***
  //I need to collect enough data that I can fit it to the unknowns***
  //The unknowns: base linear heating rate
  //         and  temp vs change in "x seconds on = y increase"
  // need a factor that decreases the linearness
  // wait, so am I at first just trying to measure this drop off
  // then try to compensate for it?
  //***
  struct cycle plate;

  //////////NEED TO ADD IF STATEMENT TO CHECK FOR (within range of) MAX TEMP//////////
  // don't forget about the temp lag!
  float const TEMP_LAG = 0.0; // <- will this be fix or change w/temp
  if (reading.temp1 > maxTemp + TEMP_LAG || reading.temp2 > maxTemp + TEMP_LAG) {
    // KILL THE HOT PLATE
    // HOW???
  }

  // 1000 ms of hot plate on = ??? K increase
  // this is used as a conversion between heating rate and IO time
  float convFact = 1.0;

  // calculating flat rate
  float cycleRate = heatingRate / convFact;
  plate.timeOn = int(cycleRate * 1000);
  plate.timeOff = 1000 - plate.timeOn;
  //////////NEED TO ADD NOTE EXPLAINING WHAT'S THE MAXIMUM RATE//////////
  // trade off betwen longer cycles which are on longer
  // vs shorter cycles which are on shorter

  // after ??? degrees, this linear rate drops off
  // for now, I will ignore the drop off
  //////////NEED TO ADD ADJUSTMENT FOR TEMP//////////

  //////////CHECK IF OUT OF BOUNDS AND ADJUST ACCORDINGLY//////////
  // probably want to return something to indicate this is occuring
  // but also correct for it in real time bc that's cool

  plate.timeOn = 500;
  plate.timeOff = 1000 - plate.timeOn;

  return plate;
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
  // Get intial temperatures and time
  // Not sure if this is necessary
  currentRead.temp1 = thermocouple1.readCelsius();
  currentRead.temp2 = thermocouple2.readCelsius();
  currentRead.timeMilliseconds = millis();
}

//===============================================================================
//  Main
//===============================================================================

void loop() {
//  // For the moment, we'll have a constant cycle
//  // But these vars will be replaced by something fancier later ;)
//  int timeOn = 200;
//  int timeOff = 2000 - timeOn;
  
  // If we're streaming
  if (daqMode == STREAM) {
    int timeMillieseconds = millis();
    
    // TEMP RECORDING
    // Periodically checking temperature
    if (timeMillieseconds - timeOfLastDAQ >= daqDelay) {
      currentRead = printTemp();
      timeOfLastDAQ = currentRead.timeMilliseconds;
    }
    
    // HOT PLATE CYCLE
    // Getting heating rate
    struct cycle plate = adjustRate(currentRead);
    
    // Switching plate on or off, if mandated by cycle times
    if (plateOn) {
      // Turn off if the plate has been on too long
      if (timeMillieseconds - timeOfLastSwitch >= plate.timeOn) {
        digitalWrite(13, LOW);
        plateOn = false;
        timeOfLastSwitch = timeMillieseconds;
      }
    } else {
      // Turn on if the plate has been off too long
      if (timeMillieseconds - timeOfLastSwitch >= plate.timeOff) {
        digitalWrite(13, HIGH);
        plateOn = true;
        timeOfLastSwitch = timeMillieseconds;
      }
    }
  }
  
  // Check if data has been sent to Arduino and respond accordingly
  // If the Python code isn't running, this'll do nothing
  if (Serial.available() > 0) {
    // Read in request
    int inByte = Serial.read();
    
    // If data is requested, fetch it and write it, or handshake
    switch(inByte) {
      case TEMP_REQUEST:
        timeOfLastDAQ = printTemp().timeMilliseconds;
        break;
      case ON_REQUEST:
        daqMode = ON_REQUEST;
        break;
      case STREAM:
        daqMode = STREAM;
        break;
      case READ_HEAT_PARAMS:
        // Read in delay, knowing it is appended with an x
        rateMaxStr = Serial.readStringUntil('x');
        // Parse string into the heating rate and max temp
        comma = rateMaxStr.indexOf(',');
        heatingRate = rateMaxStr.substring(0, comma - 1).toFloat();
        maxTemp = rateMaxStr.substring(comma + 1).toFloat();
        if (Serial.availableForWrite()) {
          // String outstr = String(heatingRate) + "," + String(maxTemp);
          String outstr = rateMaxStr; 
          Serial.println(outstr);
        }
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

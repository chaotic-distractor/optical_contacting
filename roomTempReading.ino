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

unsigned long printTemp() {
  // Read value from analog pin
  int val = analogRead(0);
  double value = Thermister(val);

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

void setup() {            //This function gets called when the Arduino starts
  Serial.begin(115200);   //This code sets up the Serial port at 115200 baud rate
}

double Thermister(int RawADC) {  //Function to perform the fancy math of the Steinhart-Hart equation
 double Temp;
 Temp = log(((10240000/RawADC) - 10000));
 Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
 Temp = Temp - 273.15;              // Convert Kelvin to Celsius
 Temp = (Temp * 9.0)/ 5.0 + 32.0; // Celsius to Fahrenheit - comment out this line if you need Celsius
 return Temp;
}

//void loop() {             //This function loops while the arduino is powered
  //int val;                //Create an integer variable
  //double temp;            //Variable to hold a temperature value
  //val=analogRead(0);      //Read the analog port 0 and store the value in val
  //temp=Thermister(val);   //Runs the fancy math on the raw analog value
  //Serial.println(temp);   //Print the value to the serial port
  //delay(100);            //Wait one tenth of a second before we do it again
//}

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

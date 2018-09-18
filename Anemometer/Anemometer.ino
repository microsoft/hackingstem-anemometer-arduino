//===----------------__ Hacking STEM – [Filename] – Arduino __------------===//
// For use with the Analyzing Wind Speed lesson plan
// available from Microsoft Education Workshop at http://aka.ms/hackingSTEM
//
// Overview:
// [Several sentences describing code function in accessible verbiage]
//
// This project uses an Arduino UNO microcontroller board, information at:
// https://www.arduino.cc/en/main/arduinoBoardUno
//
// Comments, contributions, suggestions, bug reports, and feature requests
// are welcome! For source code and bug reports see:
// http://github.com/[TODO github path to Hacking STEM]
//
// Copyright Microsoft EDU Workshop - HackingSTEM
// MIT License terms detailed in LICENSE.txt
//===----------------------------------------------------------------------===//


#include <stdlib.h>

// Variables in data format
float tempC = 0;
float tempF = 0;
float pressure = 0;
float altitudeM = 0;
float altitudeFt = 0;
float humidity = 0;
float windSpeed = 0;
int motorSpeed = 0;

// WindDriven-Anemometer------------------------------------>
float timeCurrent = 0;
float timePrevious = 0;
float timeInterval = 0;
byte switchState = 0;

float diameter = 9.44;               // diameter of anemometer in inches
float circumference = diameter * PI; // PI * D = circumference
float feet = circumference / 12;     // convert to inches
float miles = feet / 5280;           // covert to miles
float millisPerMinute = 60000;       // 1000 millisends per second * 60
float rpm = 0;                       // revolutions per minute
float mpm = 0;                       // miles per minute
float mph = 0;                       // miles per hour
float kph = 0;                       // kilometers per hour

// ExcelDriven-Anemometer---------------------------------->
String inputString = ""; // a string to hold incoming data
boolean stringComplete = false; // whether the string is
                                // completely built (newline found)


// Arduino uses setup() method to prepare things before running
// the main code (found in the loop() method.)
void setup() {
  Serial.begin(9600); // start the serial port at 9600 baud rate

  // WindDriven-Anemometer------------------------------------>
  pinMode(2, INPUT);
  timeCurrent = millis(); // initialize timeCurrent

  // Temperature-Sensor--------------------------------------->
  analogReference(5.0); // voltage
}

// Ardiuno repeatedly call the loop method(), this is generally where
// main logic of the code resides.
void loop() {
  // WindDriven-Anemometer------------------------------------>
  switchState = digitalRead(2);
  if (switchState == 1) { // enter calculations when the reed switch is activate
    timeCurrent = millis(); // set current time
    timeInterval =
        timeCurrent -
        timePrevious; // calculate interval between positive switch states
    if (timeInterval > 45) { // setting a minimum of 45ms eliminates
                             // switch bounce readings
      rpm = millisPerMinute / timeInterval; // total number of milliseconds in
                                            // a minute divided by the interval
                                            // between reads
      mpm = miles * rpm;    // miles per minute
      mph = mpm * 60;       // convert to mph
      kph = mph * 1.609344; // convert to kph
      windSpeed = kph;      // windspeed in kph

      int sensorInput = analogRead(A0); // read the analog sensor
      tempC = (double)sensorInput / 1024; // divide by 1024 to get percentage of
                                          // input reading in millivolts
      tempC = tempC * 5;                  // multiply by 5V to get voltage
      tempC = tempC - 0.5; // Subtract the offset specified in TMP36 datasheet
      tempC = tempC * 100; // Convert to degrees

      tempF = tempC * 9 / 5 + 32; // Convert to Fahrenheit

      sendSerialData(); // now that all values are calculated send serial data
                        // string to Add-In
    }
    timePrevious = timeCurrent; // set previous time for next reading
  }

  getSerialData();      // gather the input stream (from excel)

  // when the newline arrives motorspeed is 4th element (3rd index in base 0)
  if (stringComplete) {
    motorSpeed = getValue(inputString, ',', 3).toInt(); // see getValue() below
    analogWrite(9, motorSpeed);                // write out motorSpeed to pin 9
    inputString = "";                          // reset inputString
    stringComplete = false;                    // reset stringComplete flag
  }
}

// getSerialData() methods reads lines from serial input,
// which, in this application, is data from Excel workbook
void getSerialData() {
  while (Serial.available()) {
    char inChar = (char)Serial.read(); // get new byte
    inputString += inChar;             // add it to input string

    // if we get a newline we have a complete string of data to process
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

// getValue() parses a delimited string and returns the value in
// position specified by 'index' parameter
//
// parameters:
//   dataString: string of text, with some delimiter (eg "AAA,BBB,CCC")
//   separator: char representing the delimiter
//   index: int of the element to return, starting at 0
//      (eg index 2 of "AAA,BBB,CCC" is CCC)
//
// returns:
//    the element given by "index" parameter
//    "" if index doesn't an element
String getValue(String dataString, char separator, int index) {
  int matchingIndex = 0;     // no match because we are starting to look
  int strIndex[] = {0, -1};
  int maxIndex = dataString.length() - 1;

  // loop until end of array or until we find a match
  for (int i = 0; i <= maxIndex && matchingIndex <= index; i++) {
    if (dataString.charAt(i) == separator ||
        i == maxIndex) { // if we hit a comma OR we are at the end of the array
      matchingIndex++;   // increment matchingIndex to keep track of where we
                         // have looked
      strIndex[0] = strIndex[1] + 1; // set substring parameters
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return matchingIndex > index ? dataString.substring(strIndex[0], strIndex[1])
                               : ""; // if match return substring or ""
}

// Sends line of text consistsing of comma delimited strings representing
// Wind Speed, Revolutions, Time Interval, temp C, & temp F,
// format:
//    windSpeed,rpm,timeInterval,tempC,tempF
// example:
//    123,22,12,0,32
void sendSerialData() {
  // format: Wind Speed, Revolutions, Time Interval, temp C, temp F,

  Serial.print(windSpeed, 2);
  Serial.print(",");
  Serial.print(rpm);
  Serial.print(",");
  Serial.print(timeInterval);
  Serial.print(",");
  Serial.print(tempC, 2);
  Serial.print(",");
  Serial.print(tempF, 2);

  Serial.println();
}

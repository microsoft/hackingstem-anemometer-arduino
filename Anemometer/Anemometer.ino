#include <stdlib.h>

//Variables in data format
float tempC = 0;
float tempF = 0;
float pressure = 0;
float altitudeM = 0;
float altitudeFt = 0;
float humidity = 0;
float windSpeed = 0;
int motorSpeed = 0;

//WindDriven-Anemometer------------------------------------>
float timeCurrent = 0;
float timePrevious = 0;
float timeInterval = 0;
byte switchState = 0;

float diameter = 9.44;                // diameter of anemometer in inches
float circumference = diameter * PI;  // PI * D = circumference
float feet = circumference / 12;      // convert to inches
float miles = feet / 5280;            // covert to miles
float millisPerMinute = 60000;        // 1000 millisends per second * 60
float rpm = 0;                        // revolutions per minute
float mpm = 0;                        // miles per minute
float mph = 0;                        // miles per hour
float kph = 0;                        // kilometers per hour

//ExcelDriven-Anemometer---------------------------------->
String inputString = "";                    // a string to hold incoming data
boolean stringComplete = false;             // whether the string is completely built (newline found)

void setup() {
  Serial.begin(9600);                       // start the serial port at 9600 baud rate

  //WindDriven-Anemometer------------------------------------>
  pinMode(2, INPUT);
  timeCurrent = millis();                   // initialize timeCurrent

  //Temperature-Sensor--------------------------------------->
  analogReference(5.0); //voltage 
}

void loop() {
  //WindDriven-Anemometer------------------------------------>
  switchState = digitalRead(2);
  if(switchState == 1){                       // enter calculations when the reed switch is activated
    timeCurrent = millis();                   // set current time
    timeInterval = timeCurrent - timePrevious;// calculate interval between positive switch states
    if(timeInterval > 45){                    // setting a minimum of 45ms eliminates switch bounce readings
      rpm = millisPerMinute / timeInterval;   // total number of milliseconcs in a minute divided by the interval between reads
      mpm = miles * rpm;                      // miles per minute
      mph = mpm * 60;                         // convert to mph
      kph = mph * 1.609344;                   // convert to kph
      windSpeed = kph;                        // windspeed in kph

      int sensorInput = analogRead(A0);       // read the analog sensor
      tempC = (double)sensorInput / 1024;     // divide by 1024 to get percentage of input reading in millivolts
      tempC = tempC * 5;                      // multiply by 5V to get voltage
      tempC = tempC - 0.5;                    // Subtract the offset specified in TMP36 datasheet
      tempC = tempC * 100;                    // Convert to degrees 
      
      tempF = tempC * 9/5 + 32;               // Convert to Fahrenheit
        
      sendSerialData();                       // now that all values are calculated send serial data string to Add-In
    }    
    timePrevious = timeCurrent;               // set previous time for next reading
  }
  
  //ExcelDriven-Anemometer---------------------------------->
  getSerialData();                            // gather the input stream
  if (stringComplete) {                       // when the newline arrives....
    //motorspeed is the 4th element (3rd index in base 0 array)
    motorSpeed = getValue(inputString, ',', 3).toInt(); // see getValue funciton below
    analogWrite(9, motorSpeed);               // write out motorSpeed to pin 9
    inputString = "";                         // reset inputString
    stringComplete = false;                   // reset stringComplete flag
  }  
}

//ExcelDriven-Anemometer---------------------------------->
void getSerialData() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();      // get new byte
    inputString += inChar;                  // add it to input string
    if (inChar == '\n') {                   // if we get a newline... 
      stringComplete = true;                // we have a complete string of data to process
    }
  }
}

//Serial input array 
String getValue(String dataString, char separator, int index)
{                                           // basic searching algorithm
                                            // data is the serial string, separator is a comma, index is where we want to look in the data array
  int matchingIndex = 0;                    // no match because we are starting to look
  int strIndex[] = {0, -1};
  int maxIndex = dataString.length()-1;
  for(int i=0; i<=maxIndex && matchingIndex<=index; i++){     // loop until end of array or until we find a match
    if(dataString.charAt(i)==separator || i==maxIndex){             // if we hit a comma OR we are at the end of the array
      matchingIndex++;                                        // increment matchingIndex to keep track of where we have looked
      strIndex[0] = strIndex[1]+1;                            // set substring parameters
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return matchingIndex>index ? dataString.substring(strIndex[0], strIndex[1]) : ""; // if match return substring or ""
}

void sendSerialData() {
    //format: Wind Speed, Revolutions, Time Interval, temp C, temp F,

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

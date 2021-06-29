/*
 Example using the SparkFun HX711 breakout board with a scale
 By: Nathan Seidle
 SparkFun Electronics
 Date: November 19th, 2014
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 This is the calibration sketch. Use it to determine the calibration_factor that the main example uses. It also
 outputs the zero_factor useful for projects that have a permanent mass on the scale in between power cycles.
 
 Setup your scale and start the sketch WITHOUT a weight on the scale
 Once readings are displayed place the weight on the scale
 Press +/- or a/z to adjust the calibration_factor until the output readings match the known weight
 Use this calibration_factor on the example sketch
 
 This example assumes pounds (lbs). If you prefer kilograms, change the Serial.print(" lbs"); line to kg. The
 calibration factor will be significantly different but it will be linearly related to lbs (1 lbs = 0.453592 kg).
 
 Your calibration factor may be very positive or very negative. It all depends on the setup of your scale system
 and the direction the sensors deflect from zero state
 This example code uses bogde's excellent library:"https://github.com/bogde/HX711"
 bogde's library is released under a GNU GENERAL PUBLIC LICENSE
 Arduino pin 2 -> HX711 CLK
 3 -> DOUT
 5V -> VCC
 GND -> GND

 Most any pin on the Arduino Uno will be compatible with DOUT/CLK.

 The HX711 board can be powered from 2.7V to 5V so the Arduino 5V power should be fine.


How to calibrate your load cell
1.Call set_scale() with no parameter.
2.Call tare() with no parameter.
3.Place a known weight on the scale and call get_units(10).
4.Divide the result in step 3 to your known weight. You should get about the parameter you need to pass to set_scale().
5.Adjust the parameter in step 4 until you get an accurate reading

*/

#include "HX711.h"

#define LOADCELL_DOUT_PIN  3
#define LOADCELL_SCK_PIN  2

HX711 scale;

float calibration_factor = -380; //-7050 worked for my 440lb max scale setup

// READING SMOOTHING

const int numReadings = 20;
float readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
float total = 0;                  // the running total
float average = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  
  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
}

void loop() {
  total = total - readings[readIndex];   // subtract the last reading:
  readings[readIndex] = scale.get_units();  // read from the sensor:
  total = total + readings[readIndex];   // add the reading to the total:
  readIndex = readIndex + 1;   // advance to the next position in the array:

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
      readIndex = 0;    // ...wrap around to the beginning:
    }

  average = round(total / numReadings);   // calculate the average (of sensor readings)

  
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  Serial.print("Reading: ");
  Serial.print((int)average);
  Serial.print(" g"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
  Serial.print(" calibration_factor: ");
  Serial.print(calibration_factor);
  Serial.println();

  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 1;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 1;
    else if(temp == '0')
      scale.tare();
  }
}

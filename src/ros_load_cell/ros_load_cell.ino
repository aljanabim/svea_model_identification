/*
  Code for taking readings force readings from an HX711 load cell and pushing
  the data to a ROS topic.
  By: Mustafa Al-Janabi
  License: MIT

  Connect the load cell to the arduino according to the following

  (orange)    -   VCC
  (yellow)    -   GND
  DT (red)    -   PIN 3
  SCK (brown) -   PIN 2


  Connect arduino to SVEA on port 6 in the usb hub
  --> port=/dev/ttyACM1


  How to calibrate your load cell
  1.Call set_scale() with no parameter.
  2.Call tare() with no parameter.
  3.Place a known weight on the scale and call get_units(10).
  4.Divide the result in step 3 to your known weight. You should get about the parameter you need to pass to set_scale().
  5.Adjust the parameter in step 4 until you get an accurate reading
*/

#include <ros.h>
#include <std_msgs/Int8.h>
#include <sensor_msgs/Temperature.h>
#include "HX711.h" // By bogde

// ------------------------------------
// LOAD CELL VARIABLES AND CONSTANTS
// ------------------------------------

#define LOADCELL_DOUT_PIN  3
#define LOADCELL_SCK_PIN  2

HX711 scale;
unsigned long start_time;
unsigned long elapsed_time;

int calibration_factor = -375; //-7050 worked for my 440lb max scale setup

// READING SMOOTHING

const int numReadings = 5;
float readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
float total = 0;                  // the running total
float average = 0;

// ---------------------------
// ROS VARIABLES AND CONSTANTS
// ---------------------------

ros::NodeHandle  nh;

// Setup subscriber for load cell commands 
// (such as tare, increase/decrease calibration factor)
std_msgs::Int8 cmd_msg;
void loadCellCmdCb( const std_msgs::Int8& cmd_msg){
  switch(cmd_msg.data){
    case 0:
      scale.tare();
      break;
    case 1:
      calibration_factor += 1;
      break;
    case -1:
      calibration_factor -= 1;
      break;
    default:
      break;
  }
}
ros::Subscriber<std_msgs::Int8> load_cell_sub("load_cell_cmd", loadCellCmdCb);

// Setup publisher for load cell readings
sensor_msgs::Temperature sensor_msg;
ros::Publisher load_cell_pub("load_cell", &sensor_msg);

char frame_id[] = "base_link";

void setup()
{
  // SETUP LOAD CELL
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0

  // SETUP ROS
  nh.initNode();
  nh.advertise(load_cell_pub);
  nh.subscribe(load_cell_sub);
  sensor_msg.header.frame_id = frame_id;
}

void loop()
{
  // GET READINGS FROM LOAD CELL AND SMOOTH THEM
  total = total - readings[readIndex];   // subtract the last reading:
  readings[readIndex] = scale.get_units();  // read from the sensor:
  total = total + readings[readIndex];   // add the reading to the total:
  readIndex = readIndex + 1;   // advance to the next position in the array:

  if (readIndex >= numReadings) { // if we're at the end of the array...
    readIndex = 0;    // ...wrap around to the beginning:
  }

  average = round(total / numReadings); // calculate the average (of sensor readings)
  scale.set_scale(calibration_factor); //Adjust to this calibration factor

  // PUBLISH THE READINGS TO ROS 
  sensor_msg.header.stamp = nh.now();
  sensor_msg.temperature = average;
  load_cell_pub.publish( &sensor_msg );
  nh.spinOnce();
}

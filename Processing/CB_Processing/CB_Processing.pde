// Gets OSC data from Wekinator and sends to bluetooth
// Adapted from code by Rebecca Fiebrink: October 2017

import processing.serial.*;
import controlP5.*;
import java.util.*;
import oscP5.*;
import netP5.*;

//Objects for display:
ControlP5 cp5;
PFont fBig;
CColor defaultColor;

//Serial port info:
int end = 10;    // the number 10 is ASCII for linefeed (end of serial.println), later we will look for this to break up individual messages
String serial;   // declare a new string called 'serial' . A string is a sequence of characters (data type know as "char")
int numPorts = 0;
Serial myPort;  // The serial port
boolean gettingData = false; //True if we've selected a port to read from

//Objects for sending OSC
OscP5 oscP5;
NetAddress dest;

int numFeatures = 0;
String featureString = "";

void setup() {
  size(300, 250);
  frameRate(100);

  //Set up display
  cp5 = new ControlP5(this);
  textAlign(LEFT, CENTER);
  fBig = createFont("Arial", 12);

  //Populate serial port options:
  List l = Arrays.asList(Serial.list());
  numPorts = l.size();
  cp5.addScrollableList("Port") //Create drop-down menu
     .setPosition(10, 60)
     .setSize(200, 100)
     .setBarHeight(20)
     .setItemHeight(20)
     .addItems(l)
     ;
  defaultColor = cp5.getColor();
     
  //Set up OSC:
  oscP5 = new OscP5(this,12000); //listen for OSC messages on port 12000 (Wekinator default)
  dest = new NetAddress("127.0.0.1",6448); //Send to port 6448
}

//Called when new port (n-th) selected in drop-down
void Port(int n) {
 // println(n, cp5.get(ScrollableList.class, "Port").getItem(n));
  CColor c = new CColor();
  c.setBackground(color(255,0,0));
  
  //Color all non-selected ports the default color in drop-down list
  for (int i = 0; i < numPorts; i++) {
      cp5.get(ScrollableList.class, "Port").getItem(i).put("color", defaultColor);
  }
  
  //Color the selected item red in drop-down list
  cp5.get(ScrollableList.class, "Port").getItem(n).put("color", c);
  
  //If we were previously receiving on a port, stop receiving
  if (gettingData) {
    myPort.stop();
  }
  
  //Finally, select new port:
  myPort = new Serial(this, Serial.list()[n], 9600); //Using 9600 baud rate
  myPort.clear(); //Throw out first reading, in case we're mid-feature vector
  gettingData = true;
  serial = null; //Initialise serial string
  numFeatures = 0;
}

//Called in a loop at frame rate (100 Hz)
void draw() {

}


//called automatically when OSC message is received
void oscEvent(OscMessage theOscMessage) { 
  try {
  if (myPort.available() > 0) {
 if (theOscMessage.checkAddrPattern("/output_1")==true) {
        println("1");
        myPort.write(1);  
 } else if (theOscMessage.checkAddrPattern("/output_2")==true) {
     println("2");
     myPort.write(2);  
 } else if (theOscMessage.checkAddrPattern("/output_3") == true) {
     println("3");
     myPort.write(3);  
 }
}} catch (Exception e) {
}
}
// Obstacle avoidance code loosely based on: http://mertarduinotutorial.blogspot.co.uk/2017/03/arduino-project-tutorial-17-obstacle.html
// The rest is mine.

#include <Servo.h>
#include <NewPing.h>

const int LeftMotorForward = 7;
const int LeftMotorBackward = 6;
const int RightMotorForward = 4;
const int RightMotorBackward = 5;

//for determinind loudest sound
const int threshold = 10;

//for sound calibration
int cr;
int cl;
int cf;

//analog input pins for sound input
const int front_sound = A3;
const int right_sound = A4;
const int left_sound = A5;

// setting up ping sensor
#define trig_pin A0 //analog input 0
# define echo_pin A1 //analog input 1
# define maximum_distance 200
int distance = 100;
NewPing sonar(trig_pin, echo_pin, maximum_distance);

Servo servo_motor;

//keeps track of whether we're moving forward - don't want to send high output to motors multiple times
boolean movingForward = false;

//keeps track of whether the bot is waiting or not
boolean waiting = false;

void setup() {
  // start serial
  Serial.begin(9600);

  // set pin modes for motor output
  pinMode(RightMotorForward, OUTPUT);
  pinMode(LeftMotorForward, OUTPUT);
  pinMode(LeftMotorBackward, OUTPUT);
  pinMode(RightMotorBackward, OUTPUT);

  //attach servo to pin 10
  servo_motor.attach(10);

  //point servo forwards
  servo_motor.write(115);
  delay(500);

  //read initial pings
  distance = readPing();
  delay(100);

  //calibrate sound input
  int base_r = 0;
  int base_l = 0;
  int base_f = 0;

  // sample sound environment when bot is started in case the room is noisy
  for (int i = 0; i < 100; i++) {
    int right = analogRead(right_sound);
    base_r = base_r + right;
    int left = analogRead(left_sound);
    base_l = base_l + left;
    int front = analogRead(front_sound);
    base_f = base_f + front;
  }
  cr = base_r / 100;
  cl = base_l / 100;
  cf = base_f / 100;
  //so now any sound input minus calibration number (e.g. right sound - cr) should be around zero-ish

}

void loop() {

  // listen for commands and noises
  listen();
  if (!waiting) {
    explore();
  }
}

void listen() {

  Serial.println("Listening...");

  // listen for commands from wekinator
  int weki_class = 0;
  if (Serial.available() > 0) {
    // read next available byte from serial
    weki_class = (int) Serial.read();

    //if command is "stop"
    if (weki_class == 1) {
      moveStop();
      waiting = true;
    }

    //if command is "go"
    if (weki_class == 2) {
      //just carry on
      if (waiting) {
        waiting = false;
      }
    }

    //if command is whistle
    if (weki_class == 3) {
      //whistle
      turnAround();
    }
    /*
    could have extra ifs here for more commands
    */
  }
  // if there was no command from wekinator and not waiting, respond instead to sound environment
  else if (!waiting) {
    // if no commands from wekinator and not waiting
    // listen to environment 
    int r = analogRead(right_sound);
    int l = analogRead(left_sound);
    int f = analogRead(front_sound);

    Serial.print("Sound input right, left, front: ");
    Serial.print(r, DEC);
    Serial.print(",");
    Serial.print(l, DEC);
    Serial.print(",");
    Serial.print(f, DEC);
    Serial.println();

    // if loudest noise from behind
    if (r - threshold >= f && l - threshold >= f) {
      turnAround();
      delay(200);
    }
    // loudest noise from right
    else if (r - threshold >= l && r - threshold >= f) {
      turnRight();
      delay(200);
    }
    //loudest noise from left
    else if (l - threshold >= r && l - threshold >= f) {
      turnLeft();
      delay(200);
    }
  }
}

void turnAround() {

  Serial.println("Turning around...");

  digitalWrite(LeftMotorForward, HIGH);
  digitalWrite(RightMotorBackward, HIGH);

  digitalWrite(LeftMotorBackward, LOW);
  digitalWrite(RightMotorForward, LOW);

  delay(1000);

  digitalWrite(LeftMotorForward, HIGH);
  digitalWrite(RightMotorForward, HIGH);

  digitalWrite(LeftMotorBackward, LOW);
  digitalWrite(RightMotorBackward, LOW);

}

void explore() {

  Serial.println("Exploring...");
  moveForward();
  checkForObstacles();

}

void checkForObstacles() {

  Serial.println("Checking for obstacles...");

  // read distance in front 
  distance = readPing();
  int distanceRight = 0;
  int distanceLeft = 0;
  delay(50);

  if (distance <= 30) {

    Serial.println("Obstacle!");

    // look both ways and see where there is more room
    moveStop();
    delay(300);
    moveBackward();
    delay(400);
    moveStop();
    delay(300);
    distanceRight = lookRight();
    delay(300);
    distanceLeft = lookLeft();
    delay(300);

    if (distance >= distanceLeft) {
      turnRight();
      moveStop();
    } else {
      turnLeft();
      moveStop();
    }
  }
}

int lookRight() {

  Serial.println("Looking right...");

  // look right
  servo_motor.write(50);
  delay(500);
  // get distance 
  int distance = readPing();
  delay(100);
  // turn servo to front
  servo_motor.write(115);
  return distance;
}

int lookLeft() {

  Serial.println("Looking left...");

  // look left
  servo_motor.write(170);
  delay(500);
  int distance = readPing();
  delay(100);
  // turn servo to front
  servo_motor.write(115);
  return distance;
}

int readPing() {

  Serial.println("Reading ping...");

  delay(70);
  int cm = sonar.ping_cm();
  if (cm == 0) {
    cm = 250;
  }
  return cm;
}

void moveStop() {

  Serial.println("Stopping...");

  movingForward = false;
  digitalWrite(RightMotorForward, LOW);
  digitalWrite(LeftMotorForward, LOW);
  digitalWrite(RightMotorBackward, LOW);
  digitalWrite(LeftMotorBackward, LOW);
}

void moveForward() {

  // if not moving forward already
  if (!movingForward) {

    movingForward = true;

    Serial.println("Moving forward...");

    digitalWrite(RightMotorForward, HIGH);
    digitalWrite(LeftMotorForward, HIGH);
    digitalWrite(LeftMotorBackward, LOW);
    digitalWrite(RightMotorBackward, LOW);
  }
}

void moveBackward() {

  Serial.println("Moving backward...");

  movingForward = false;

  digitalWrite(LeftMotorBackward, HIGH);
  digitalWrite(RightMotorBackward, HIGH);
  digitalWrite(LeftMotorForward, LOW);
  digitalWrite(RightMotorForward, LOW);

}

void turnRight() {

  Serial.println("Turning right...");

  digitalWrite(LeftMotorForward, HIGH);
  digitalWrite(RightMotorBackward, HIGH);
  digitalWrite(LeftMotorBackward, LOW);
  digitalWrite(RightMotorForward, LOW);

  delay(500);

  digitalWrite(LeftMotorForward, HIGH);
  digitalWrite(RightMotorForward, HIGH);
  digitalWrite(LeftMotorBackward, LOW);
  digitalWrite(RightMotorBackward, LOW);

}

void turnLeft() {

  Serial.println("Turning left...");
  digitalWrite(LeftMotorBackward, HIGH);
  digitalWrite(RightMotorForward, HIGH);
  digitalWrite(LeftMotorForward, LOW);
  digitalWrite(RightMotorBackward, LOW);

  delay(500);

  digitalWrite(LeftMotorForward, HIGH);
  digitalWrite(RightMotorForward, HIGH);
  digitalWrite(LeftMotorBackward, LOW);
  digitalWrite(RightMotorBackward, LOW);

}
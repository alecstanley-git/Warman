#include <Arduino.h>

// <WM1 PIN DEFINITIONS>
#define M1_PWM 6
#define M1_DIR 7

#define M2_PWM 3
#define M2_DIR 8

// defining on/off states for each motor direction for easily readable code
#define forward1 HIGH
#define forward2 LOW
#define backward1 LOW
#define backward2 HIGH

/* unused pins
#define M3_PWM 5
#define M3_DIR 12

#define M4_PWM 11
#define M4_DIR 13

#define RELAY1 2
#define RELAY2 4

#define SERVO1 9
#define SERVO2 10
*/

/* <WIRING DESCRIPTION>                        
TBC
*/

/* <FUNCTIONALITY DESCRIPTION>
TBC
*/

void setup() {
  // CONFIGURE ALL WM1 PINS AS OUTPUTS
  pinMode(M1_PWM, OUTPUT);
  pinMode(M1_DIR, OUTPUT);
  pinMode(M2_PWM, OUTPUT);
  pinMode(M2_DIR, OUTPUT);

  /* unused pins:
  pinMode(M3_PWM, OUTPUT);
  pinMode(M3_DIR, OUTPUT);
  pinMode(M4_PWM, OUTPUT);
  pinMode(M4_DIR, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(SERVO1, OUTPUT);
  pinMode(SERVO2, OUTPUT);
  */
}

void loop() {
  // DIR pin dictates motor's dirrection
  // PWM pin dictates the motor's voltage (0-255), which loosely controls speed

  moveForward(4,100);
  moveForward(4,50);
  turnRight(45);
  moveForward(8,100);

}

// <FUNCTIONS>

void moveForward(double seconds, int percentagePower) {
  int time = (int) seconds * 1000; // convert seconds to ms
  double powerMultiplier = (double) percentagePower / 100; // convert percentage to decimal

  int power = (int) powerMultiplier * 255;

  // set direction of both wheels to forward
  digitalWrite(M1_DIR, forward1);
  digitalWrite(M2_DIR, forward2);


  analogWrite(M1_PWM, power);
  analogWrite(M2_PWM, power);

  delay(time);

  analogWrite(M1_PWM, 0);
  analogWrite(M2_PWM, 0);
}

void moveBackward(double seconds, int percentagePower) {
  int time = (int) seconds * 1000; // convert seconds to ms
  double powerMultiplier = (double) percentagePower / 100; // convert percentage to decimal

  int power = (int) powerMultiplier * 255;

  // set direction of both wheels to backward
  digitalWrite(M1_DIR, backward1);
  digitalWrite(M2_DIR, backward2);


  analogWrite(M1_PWM, power);
  analogWrite(M2_PWM, power);

  delay(time);

  analogWrite(M1_PWM, 0);
  analogWrite(M2_PWM, 0);
}

void turnLeft(int angle) {

  // approximate turning time from angle (needs tuning)
  int time = angle*10;

  // rotation caused by opposite directions
  digitalWrite(M1_DIR, forward1);
  digitalWrite(M2_DIR, backward2);

  delay(time);

  analogWrite(M1_PWM, 0);
  analogWrite(M2_PWM, 0);

}

void turnRight(int angle) {

  // approximate turning time from angle (needs tuning)
  int time = angle*10;

  // rotation caused by opposite directions
  digitalWrite(M1_DIR, backward1);
  digitalWrite(M2_DIR, forward2);

  delay(time);

  analogWrite(M1_PWM, 0);
  analogWrite(M2_PWM, 0);

}

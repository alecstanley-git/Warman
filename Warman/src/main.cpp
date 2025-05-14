#include <Arduino.h>

// 11:07am

// <WM1 PIN DEFINITIONS>
#define M1_PWM 6
#define M1_DIR 7

#define M2_PWM 3
#define M2_DIR 8

#define M3_PWM 5
#define M3_DIR 12

#define M4_PWM 11
#define M4_DIR 13

// M5 is the overhead lifting pulley
#define M5_pin1 22
#define M5_pin2 24

// M6 is the arm extension pulley
#define M6_pin1 26
#define M6_pin2 28

#define uLeftTrig 46
#define uLeftEcho 48

#define uRightTrig 47
#define uRightEcho 49

#define buttonPin 50

#define redLEDPin 53
#define greenLEDPin 52

// --- Constants ---
const int NUM_SAMPLES = 5;
const float CLIFF_THRESHOLD = 20.0; // cm

// --- Moving Average Buffers ---
float distLeftBuf[NUM_SAMPLES] = {0};
float distRightBuf[NUM_SAMPLES] = {0};
int sampleIndex = 0;

void setup() {
  Serial.begin(9600);
  // CONFIGURE ALL WM1 PINS AS OUTPUTS
  pinMode(M1_PWM, OUTPUT);
  pinMode(M1_DIR, OUTPUT);
  pinMode(M2_PWM, OUTPUT);
  pinMode(M2_DIR, OUTPUT);
  pinMode(M3_PWM, OUTPUT);
  pinMode(M3_DIR, OUTPUT);
  pinMode(M4_PWM, OUTPUT);
  pinMode(M4_DIR, OUTPUT);
  pinMode(M5_pin1, OUTPUT);
  pinMode(M5_pin2, OUTPUT);
  pinMode(M6_pin1, OUTPUT);
  pinMode(M6_pin2, OUTPUT);

  // Ultrasonic pins
  pinMode(uLeftTrig, OUTPUT);
  pinMode(uRightTrig, OUTPUT);
  pinMode(uLeftEcho, INPUT);
  pinMode(uRightEcho, INPUT);

  // Misc pins
  pinMode(buttonPin, INPUT);
  pinMode(greenLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);

}

// Wait for button press upon start
void buttonWait(int pin) {
  while (digitalRead(pin) == LOW) {
    delay(10);
  }
  return;
}

// Define function to stop all motors to avoid repetition in code
void stopMotors() {

  analogWrite(M1_PWM, 0);
  analogWrite(M2_PWM, 0);
  analogWrite(M3_PWM, 0);
  analogWrite(M4_PWM, 0);
}

void moveForward(int milliseconds, int power) {

  // HIGH and LOW set according to mecanum wheel parameters
  digitalWrite(M1_DIR, HIGH);
  digitalWrite(M2_DIR, HIGH);
  digitalWrite(M3_DIR, LOW);
  digitalWrite(M4_DIR, LOW);

  analogWrite(M1_PWM, power);
  analogWrite(M2_PWM, power);
  analogWrite(M3_PWM, power);
  analogWrite(M4_PWM, power);

  delay(milliseconds);

  stopMotors();
}

// Function called when rover is edging towards the end of the board, to allow it to split left and right motion
void moveHalfForward(int milliseconds, int power, int side) {

  if (side == 0) { // LEFT SIDE
    digitalWrite(M1_DIR, LOW);
    digitalWrite(M2_DIR, LOW);

    analogWrite(M1_PWM, power);
    analogWrite(M2_PWM, power);

    delay(milliseconds);
    stopMotors();

  } else if (side == 1) { // RIGHT SIDE
    digitalWrite(M3_DIR, LOW);
    digitalWrite(M4_DIR, LOW);

    analogWrite(M3_PWM, power);
    analogWrite(M4_PWM, power);

    delay(milliseconds);
    stopMotors();
  }
}

void moveBackward(int milliseconds, int power) {
  digitalWrite(M1_DIR, LOW);
  digitalWrite(M2_DIR, LOW);
  digitalWrite(M3_DIR, HIGH);
  digitalWrite(M4_DIR, HIGH);

  analogWrite(M1_PWM, power);
  analogWrite(M2_PWM, power);
  analogWrite(M3_PWM, power);
  analogWrite(M4_PWM, power);

  delay(milliseconds);

  stopMotors();
}

void strafeLeft(int milliseconds, int power) {
  digitalWrite(M1_DIR, LOW);  // FL Backward
  digitalWrite(M2_DIR, HIGH); // RL Forward
  digitalWrite(M3_DIR, HIGH); // FR Forward
  digitalWrite(M4_DIR, LOW);  // RR Backward

  analogWrite(M1_PWM, power);
  analogWrite(M2_PWM, power);
  analogWrite(M3_PWM, power);
  analogWrite(M4_PWM, power);

  delay(milliseconds);

  stopMotors();
}

void strafeRight(int milliseconds, int power) {
  digitalWrite(M1_DIR, HIGH); // FL Forward
  digitalWrite(M2_DIR, LOW);  // RL Backward
  digitalWrite(M3_DIR, LOW);  // FR Backward
  digitalWrite(M4_DIR, HIGH); // RR Forward

  analogWrite(M1_PWM, power);
  analogWrite(M2_PWM, power);
  analogWrite(M3_PWM, power);
  analogWrite(M4_PWM, power);

  delay(milliseconds);

  stopMotors();
}

// Turn robot around after strafing across board
// Function works by using wheels in particular motion for a set period of time
void flip180() {
  digitalWrite(M1_DIR, HIGH);
  digitalWrite(M2_DIR, HIGH);
  digitalWrite(M3_DIR, HIGH);
  digitalWrite(M4_DIR, HIGH);

  analogWrite(M1_PWM, 0);
  analogWrite(M2_PWM, 255);
  analogWrite(M3_PWM, 255);
  analogWrite(M4_PWM, 0);

  delay(14700);

  stopMotors();
}

void raiseArm(int milliseconds) {

  digitalWrite(M5_pin1, HIGH);
  digitalWrite(M5_pin2, LOW);

  delay(milliseconds);
  digitalWrite(M5_pin1, LOW);
  digitalWrite(M5_pin2, LOW);
}

void lowerArm(int milliseconds) {

  digitalWrite(M5_pin1, LOW);
  digitalWrite(M5_pin2, HIGH);

  delay(milliseconds);
  digitalWrite(M5_pin1, LOW);
  digitalWrite(M5_pin2, LOW);
}

void extendArm(int milliseconds) {

  digitalWrite(M6_pin1, HIGH);
  digitalWrite(M6_pin2, LOW);

  delay(milliseconds);
  digitalWrite(M6_pin1, LOW);
  digitalWrite(M6_pin2, LOW);
}

void retractArm(int milliseconds) {

  digitalWrite(M6_pin1, LOW);
  digitalWrite(M6_pin2, HIGH);

  delay(milliseconds);
  digitalWrite(M6_pin1, LOW);
  digitalWrite(M6_pin2, LOW);
}

// Distance function works by pulsing the ultrasonic sensor and calibrating the time output into centimetres.
// Included a catch for 0 "infinite" values
float readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    return 999.0; // if echo times out
  }

  return duration * 0.034 / 2;
}

// Averages the last five ultrasonic samples to avoid false positives
float computeAverage(float buffer[]) {
  float sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += buffer[i];
  }
  return sum / NUM_SAMPLES;
}

void loop() {

  // Show that rover is ready to start
  digitalWrite(redLEDPin,HIGH);

  // Wait for button press
  buttonWait(buttonPin);

  digitalWrite(redLEDPin,LOW); // Turn off start LED

  delay(1000);

  extendArm(6000);

  raiseArm(2500);

  // Approach the balls
  moveForward(8000,150);
  
  delay(1000);

  //
  // ARM OPERATION TO PICKUP BALLS
  //

  // Approach the balls
  moveForward(2000,150);

  // Lower the arm over the balls
  lowerArm(1800); // lowering requires less time as the downwards weight has less torque, motor can operate faster

  // Retract arm
  retractArm(6000);

  // Raise arm with balls inside it as high as possible to keep centre of mass as close as possible
  // raiseArm(5000);

  // Approach the edge of the board, but not too close. Next section handles approach
  moveForward(3000,150);

  //
  // APPROACHING EDGE OF BOARD (USING ULTRASONICS)
  //

  // Initialising distance buffers
  for (int i = 0; i < NUM_SAMPLES; i++) {
    distLeftBuf[i] = readDistance(uLeftTrig, uLeftEcho);
    distRightBuf[i] = readDistance(uRightTrig, uRightEcho);
    // Add a small delay if needed, e.g., if sensors interfere or need settling time
    delay(60); // HC-SR04 needs ~60ms between measurements
  }

  // Initialise loop conditions, index and boolean
  sampleIndex = 0;
  bool approach = true;

  // Loop will only stop once both sensors read >20cm (at the edge)
  while(approach) {

    // Read distances
    float currentLeft = readDistance(uLeftTrig, uLeftEcho);
    float currentRight = readDistance(uRightTrig, uRightEcho);

    // Store into buffers
    distLeftBuf[sampleIndex] = currentLeft;
    distRightBuf[sampleIndex] = currentRight;
    sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;

    // Compute smoothed averages
    float avgLeft = computeAverage(distLeftBuf);
    float avgRight = computeAverage(distRightBuf);

    // Detect cliff with averaged values
    bool cliffLeft = avgLeft > CLIFF_THRESHOLD;
    bool cliffRight = avgRight > CLIFF_THRESHOLD;

    // Move each side forward if safe to do so, otherwise stop
    if (!cliffLeft and !cliffRight) {
      moveForward(60,150); // only by a small amount

    } else if (!cliffLeft and cliffRight) {
      moveHalfForward(60,150,0);

    } else if (cliffLeft and !cliffRight) {
      moveHalfForward(60,150,1);

    } else {
      approach = false; // loop will break by setting loop condition here to false. i.e. rover is at edge
    }

  }

  delay(2000);

  //
  // DEPOSIT SEQUENCE
  //
  
  // Maneuver left across entire board, all the way into the deposit zone
  strafeLeft(5000, 200);

  moveBackward(250,150);

  strafeLeft(28000, 200);

  // Fall back from the edge
  moveBackward(5000,150);

  // Turn robot around
  flip180();

  // Approach drop zone
  moveForward(3000,150);

  //
  // BALL DROP SEQUENCE
  //

  raiseArm(4500); // Lower over drop zone

  // Let balls loose
  extendArm(5000);
  retractArm(5000);

  // Move backward and set "finished" light on
  moveBackward(2000,150);
  digitalWrite(greenLEDPin,HIGH);

  delay(20000);
}
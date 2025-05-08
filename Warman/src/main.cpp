#include <Arduino.h>

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

#define uLeftTrig 31
#define uLeftEcho 30

#define uRightTrig 43
#define uRightEcho 45

#define buttonPin 38

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

  pinMode(uLeftTrig, OUTPUT);
  pinMode(uRightTrig, OUTPUT);
  pinMode(uLeftEcho, INPUT);
  pinMode(uRightEcho, INPUT);

}

void buttonWait(int pin) {
  while (digitalRead(pin) == LOW) {
    delay(10);
  }
  return;
}

void moveForward(int milliseconds, int power) {
  digitalWrite(M1_DIR, HIGH);
  digitalWrite(M2_DIR, HIGH);
  digitalWrite(M3_DIR, HIGH);
  digitalWrite(M4_DIR, HIGH);

  analogWrite(M1_PWM, power);
  analogWrite(M2_PWM, power);
  analogWrite(M3_PWM, power);
  analogWrite(M4_PWM, power);

  delay(milliseconds);

  analogWrite(M1_PWM, 0);
  analogWrite(M2_PWM, 0);
  analogWrite(M3_PWM, 0);
  analogWrite(M4_PWM, 0);
}

void moveHalfForward(int milliseconds, int power, int side) {
  if (side == 0) { // LEFT SIDE
    digitalWrite(M1_DIR, HIGH);
    digitalWrite(M2_DIR, HIGH);
    analogWrite(M1_PWM, power);
    analogWrite(M2_PWM, power);
    delay(milliseconds);
    analogWrite(M1_PWM, 0);
    analogWrite(M2_PWM, 0);
  } else if (side == 1) { // RIGHT SIDE
    digitalWrite(M3_DIR, HIGH);
    digitalWrite(M4_DIR, HIGH);
    analogWrite(M3_PWM, power);
    analogWrite(M4_PWM, power);
    delay(milliseconds);
    analogWrite(M3_PWM, 0);
    analogWrite(M4_PWM, 0);
  }
}

void moveBackward(int milliseconds, int power) {
  digitalWrite(M1_DIR, LOW);
  digitalWrite(M2_DIR, LOW);
  digitalWrite(M3_DIR, LOW);
  digitalWrite(M4_DIR, LOW);

  analogWrite(M1_PWM, power);
  analogWrite(M2_PWM, power);
  analogWrite(M3_PWM, power);
  analogWrite(M4_PWM, power);

  delay(milliseconds);

  analogWrite(M1_PWM, 0);
  analogWrite(M2_PWM, 0);
  analogWrite(M3_PWM, 0);
  analogWrite(M4_PWM, 0);
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

  analogWrite(M1_PWM, 0);
  analogWrite(M2_PWM, 0);
  analogWrite(M3_PWM, 0);
  analogWrite(M4_PWM, 0);
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

  analogWrite(M1_PWM, 0);
  analogWrite(M2_PWM, 0);
  analogWrite(M3_PWM, 0);
  analogWrite(M4_PWM, 0);
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

float computeAverage(float buffer[]) {
  float sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += buffer[i];
  }
  return sum / NUM_SAMPLES;
}

void loop() {

  // wait for activation
  buttonWait(buttonPin);
  delay(2000);

  // Move forward for 10 seconds:
  moveForward(10000,255);

  // ARM OPERATION
  delay(1000);
  // raise the arm above and over the balls
  raiseArm(2000);
  // extend arm
  extendArm(5000);
  // approach the balls
  moveForward(2000,100);
  // lower the arm over the balls
  lowerArm(1500); // lowering requires less time as the downwards weight has less torque, motor can operate faster
  // retract arm
  retractArm(5000);

  // APPROACHING EDGE OF BOARD (USING ULTRASONICS)

  // initialising distance buffers
  for (int i = 0; i < NUM_SAMPLES; i++) {
    distLeftBuf[i] = readDistance(uLeftTrig, uLeftEcho);
    distRightBuf[i] = readDistance(uRightTrig, uRightEcho);
    // Add a small delay if needed, e.g., if sensors interfere or need settling time
    delay(60); // HC-SR04 needs ~60ms between measurements
  }
  sampleIndex = 0; // Ensure sampleIndex is reset

  bool approach = true;

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

    // move forward if safe to do so, otherwise stop
    if (!cliffLeft and !cliffRight) {
      moveForward(60,150); // only a small amount
    } else if (!cliffLeft and cliffRight) {
      moveHalfForward(60,150,0);
    } else if (cliffLeft and !cliffRight) {
      moveHalfForward(60,150,1);
    } else {
      approach = false; // loop can be broken by setting this to false (won't repeat)
    }

  }

  delay(2000);

  // MOVE ENTIRE ROBOT LEFT ACROSS BOARD
  
  strafeLeft(10000, 200);

}
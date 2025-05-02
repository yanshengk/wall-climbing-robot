#include <Servo.h>
#include <FastLED.h>

#include "StatusTracker.h"

// Define pin of components
#define FRONT_LED_PIN     5
#define REAR_LED_PIN      6
#define ESC_PIN           9
#define LEFT_MOTORS_PIN   10
#define RIGHT_MOTORS_PIN  11
// Define number of LED for each strip
#define FRONT_LED_COUNT   6
#define REAR_LED_COUNT    10
// Define ESC PWM throttle range in microseconds
#define ESC_THROTTLE_MIN  1100
#define ESC_THROTTLE_MAX  1940
// Define servo motor PWM range in microseconds
#define SERVO_MOTOR_CCW   1200
#define SERVO_MOTOR_STOP  1450
#define SERVO_MOTOR_CW    1700

CRGB frontLed[FRONT_LED_COUNT];
CRGB rearLed[REAR_LED_COUNT];

Servo esc;
Servo leftMotors;
Servo rightMotors;

StatusTracker<bool> connectionTracker;
StatusTracker<bool> frontLedTracker;
StatusTracker<int> edfPowerTracker;
StatusTracker<int> motionTracker;

void ledInit() {
  FastLED.addLeds<WS2812, FRONT_LED_PIN, GRB>(frontLed, FRONT_LED_COUNT);
  FastLED.addLeds<WS2812, REAR_LED_PIN, GRB>(rearLed, REAR_LED_COUNT);
}

void escInit() {
  esc.attach(ESC_PIN);
  esc.writeMicroseconds(ESC_THROTTLE_MIN);
}

void motorInit() {
  leftMotors.attach(LEFT_MOTORS_PIN);
  rightMotors.attach(RIGHT_MOTORS_PIN);
  leftMotors.writeMicroseconds(SERVO_MOTOR_STOP);
  rightMotors.writeMicroseconds(SERVO_MOTOR_STOP);
}

void setFrontLed(const char *content) {
  if (strcmp(content, "true") == 0) {
    for (int i = 0; i < FRONT_LED_COUNT; i++) {
      frontLed[i] = CRGB(255, 255, 255);
    }
    frontLedTracker.update(true);
  } else {
    for (int i = 0; i < FRONT_LED_COUNT; i++) {
      frontLed[i] = CRGB(0, 0, 0);
    }
    frontLedTracker.update(false);
  }
  FastLED.show();
}

void setRearLed(int r, int g, int b) {
  for (int i = 0; i < REAR_LED_COUNT; i++) {
      rearLed[i] = CRGB(r, g, b);
  }
  FastLED.show();
}

void setEdfPower(int power) {
  int input = constrain(power, 0, 100);
  int setPulseWidth = map(input, 0, 100, ESC_THROTTLE_MIN, ESC_THROTTLE_MAX);
  esc.writeMicroseconds(setPulseWidth);
  edfPowerTracker.update(input);
}

void move(const char *content) {
  if (strcmp(content, "forward") == 0) {
    leftMotors.writeMicroseconds(SERVO_MOTOR_CCW);
    rightMotors.writeMicroseconds(SERVO_MOTOR_CW);
    motionTracker.update(1);
  }
  else if (strcmp(content, "backward") == 0) {
    leftMotors.writeMicroseconds(SERVO_MOTOR_CW);
    rightMotors.writeMicroseconds(SERVO_MOTOR_CCW);
    motionTracker.update(2);
  }
  else if (strcmp(content, "left") == 0) {
    leftMotors.writeMicroseconds(SERVO_MOTOR_CW);
    rightMotors.writeMicroseconds(SERVO_MOTOR_CW);
    motionTracker.update(3);
  }
  else if (strcmp(content, "right") == 0) {
    leftMotors.writeMicroseconds(SERVO_MOTOR_CCW);
    rightMotors.writeMicroseconds(SERVO_MOTOR_CCW);
    motionTracker.update(4);
  }
  else {
    leftMotors.writeMicroseconds(SERVO_MOTOR_STOP);
    rightMotors.writeMicroseconds(SERVO_MOTOR_STOP);
    motionTracker.update(0);
  }
}

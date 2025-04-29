// #include <Arduino.h>
// #include <stdarg.h>
// #include <stdio.h>

#include <WiFiS3.h>
#include "WifiCredentials.h"
#include <WebSocketsClient.h>

#include <Arduino_LED_Matrix.h>
#include <Servo.h>

#include <ArduinoJson.h>
#include <FastLED.h>

#define PIN_ESC 9
#define PIN_LEFT_MOTORS 10
#define PIN_RIGHT_MOTORS 11
#define PIN_FRONT_LED 5
#define PIN_REAR_LED 6

#define FRONT_LED_NUM 6
#define REAR_LED_NUM 10

WebSocketsClient webSocket;

ArduinoLEDMatrix LEDMatrix;

Servo esc;
Servo leftMotors;
Servo rightMotors;

CRGB frontLed[FRONT_LED_NUM];
CRGB rearLed[REAR_LED_NUM];

const char SSID[] = WIFI_SSID;
const char PASS[] = WIFI_PASS;
int networkStatus = WL_IDLE_STATUS;

byte LEDFrame[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};



// Define EDF throttle range in microseconds
const int EDF_THROTTLE_MIN = 1100;
const int EDF_THROTTLE_MAX = 1940;
// Define servo motors PWM in microseconds
const int SERVO_MOTOR_CCW = 1200;
const int SERVO_MOTOR_STOP = 1450;
const int SERVO_MOTOR_CW = 1700;

int connectionStatus = 0;
bool frontLedStatus = 0;
int motionStatus = 0;

// This function handles WebSocket events
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  StaticJsonDocument<200> doc;
  DeserializationError err;
  const char *t;
  const char *c;

  uint8_t power;

  switch (type) {
    case WStype_DISCONNECTED:
      LEDMatrixOutput(0, 0, 0);
      Serial.println("[WSc] Disconnected!");
      connectionStatus = 0;
      break;
    case WStype_CONNECTED:
      LEDMatrixOutput(0, 0, 1);
      Serial.println("[WSc] Connected!");
      webSocket.sendTXT("ARDUINO");
      connectionStatus = 1;
      rearLED(0, 255, 0);
      break;
    case WStype_TEXT:
      Serial.print("[WSc] Received message: ");
      // 1) Null-terminate
      payload[length] = '\0';
      // 2) Parse JSON
      err = deserializeJson(doc, (char*)payload);
      if (err) {
        Serial.print("JSON parse failed: ");
        Serial.println(err.c_str());
        return;
      }
      // 3) Dispatch on "type"
      t = doc["type"];
      c = doc["content"];

      if (strcmp(t, "led") == 0) {
        frontLedStatus = !frontLedStatus;
        Serial.println(c);
        frontLED(c);
      }
      else if (strcmp(t, "motion") == 0) {
        move(c);
      }
      break;
    case WStype_BIN:
      power = payload[0];
      Serial.print("[WSc] Received power: ");
      Serial.print(power);
      Serial.println("%");
      EDF(power);
      break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}

void setup() {
  LEDMatrix.begin();

  FastLED.addLeds<WS2812, PIN_FRONT_LED, GRB>(frontLed, FRONT_LED_NUM);
  FastLED.addLeds<WS2812, PIN_REAR_LED, GRB>(rearLed, REAR_LED_NUM);

  //Initialise serial and wait for port to open
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only
  }

  Serial.flush();
  delay(1000);

  connectWifi();

  // webSocket.begin("192.168.50.137", 81);
  webSocket.begin("192.168.1.101", 81);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);

  esc.attach(PIN_ESC);
  leftMotors.attach(PIN_LEFT_MOTORS);
  rightMotors.attach(PIN_RIGHT_MOTORS);

  esc.writeMicroseconds(EDF_THROTTLE_MIN);
  leftMotors.writeMicroseconds(SERVO_MOTOR_STOP);
  rightMotors.writeMicroseconds(SERVO_MOTOR_STOP);

  delay(5000);

  LEDMatrixOutput(7, 0, 1);
}

void loop() {
  // Listen for incoming WebSocket messages
  webSocket.loop();
  updateRobotStatus();
}

void sendMessage(String type, String content) {
  String json = "{\"type\":\"" + type + "\",\"content\":\"" + content + "\"}";
  webSocket.sendTXT(json);
}

void updateRobotStatus() {
  if (connectionStatus == 1) {
    sendMessage("connection", "connected");
  } else {
    sendMessage("connection", "not connected");
  }

  if (frontLedStatus == 1) {
    sendMessage("led", "on");
  } else {
    sendMessage("led", "off");
  }

  if (motionStatus == 0) {
    sendMessage("motion", "STOP");
  }
  else if (motionStatus == 1) {
    sendMessage("motion", "FORWARD");
  }
  else if (motionStatus == 2) {
    sendMessage("motion", "BACKWARD");
  }
  else if (motionStatus == 3) {
    sendMessage("motion", "LEFT");
  }
  else if (motionStatus == 4) {
    sendMessage("motion", "RIGHT");
  }

  sendMessage("edf", String(esc.readMicroseconds()));
}

void frontLED(const char *type) {
  if (strcmp(type, "true") == 0) {
    for (int i = 0; i < FRONT_LED_NUM; i++) {
      frontLed[i] = CRGB(255, 255, 255);
    }
  } else {
    for (int i = 0; i < FRONT_LED_NUM; i++) {
      frontLed[i] = CRGB(0, 0, 0);
    }
  }
  FastLED.show();
}

void rearLED(int r, int g, int b) {
  for (int i = 0; i < REAR_LED_NUM; i++) {
      rearLed[i] = CRGB(r, g, b);
  }
  FastLED.show();
}

void move(const char *type) {
  Serial.println(type);
  if (strcmp(type, "forward") == 0) {
    motionStatus = 1;
    leftMotors.writeMicroseconds(SERVO_MOTOR_CCW);
    rightMotors.writeMicroseconds(SERVO_MOTOR_CW);
  }
  else if (strcmp(type, "backward") == 0) {
    motionStatus = 2;
    leftMotors.writeMicroseconds(SERVO_MOTOR_CW);
    rightMotors.writeMicroseconds(SERVO_MOTOR_CCW);
  }
  else if (strcmp(type, "left") == 0) {
    motionStatus = 3;
    leftMotors.writeMicroseconds(SERVO_MOTOR_CW);
    rightMotors.writeMicroseconds(SERVO_MOTOR_CW);
  }
  else if (strcmp(type, "right") == 0) {
    motionStatus = 4;
    leftMotors.writeMicroseconds(SERVO_MOTOR_CCW);
    rightMotors.writeMicroseconds(SERVO_MOTOR_CCW);
  }
  else {
    motionStatus = 0;
    leftMotors.writeMicroseconds(SERVO_MOTOR_STOP);
    rightMotors.writeMicroseconds(SERVO_MOTOR_STOP);
  }
}

void EDF(int power) {
  int input = power;
  input = constrain(input, 0, 100);
  int setPulseWidth = map(input, 0, 100, EDF_THROTTLE_MIN, EDF_THROTTLE_MAX);
  esc.writeMicroseconds(setPulseWidth);
  Serial.println(esc.readMicroseconds());
}

void LEDMatrixOutput(int row, int column, bool signal) {
  LEDFrame[row][column] = signal;
  LEDMatrix.renderBitmap(LEDFrame, 8, 12);
}

void connectWifi() {
  Serial.println();

  // Check WiFi module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed");
    // Don't continue
    while (true);
  }

  // Check WiFi firmware version
  String fv = WiFi.firmwareVersion();
  Serial.print("Current firmware version: ");
  Serial.println(fv);
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // Attempt to connect to WiFi network
  while (networkStatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(SSID);
    // Connect to WPA/WPA2 network
    networkStatus = WiFi.begin(SSID, PASS);

    // Wait 3 seconds for connection
    delay(3000);
  }

  Serial.print("Connected to network: ");
  Serial.println(WiFi.SSID());
  delay(1000);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println();
}

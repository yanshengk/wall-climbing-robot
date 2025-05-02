#include <WiFiS3.h>
#include <WebSocketsClient.h>

#include <ArduinoJson.h>

#include "Components.h"

WebSocketsClient webSocket;

const char SSID[] = "TP-Link_A86E";
const char PASS[] = "37137984";
int networkStatus = WL_IDLE_STATUS;

// This function handles WebSocket events
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  StaticJsonDocument<200> doc;
  DeserializationError err;
  const char *t;
  const char *c;

  uint8_t power;

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("[WebSocket] Disconnected!");
      connectionTracker.update(false);
      setRearLed(255, 255, 0); // Status Light: Yellow
      break;

    case WStype_CONNECTED:
      Serial.println("[WebSocket] Connected!");
      webSocket.sendTXT("ARDUINO");
      connectionTracker.update(true);
      updateRobotStatus(true);
      setRearLed(0, 255, 0); // Status Light: Green
      break;

    case WStype_TEXT:
      // Null-terminate
      payload[length] = '\0';
      // Parse JSON
      err = deserializeJson(doc, (char*)payload);
      if (err) {
        Serial.print("[WebSocket] JSON parse failed: ");
        Serial.println(err.c_str());
        return;
      }
      // Dispatch on "type"
      t = doc["type"];
      c = doc["content"];
      Serial.print("[WebSocket] Received message: ");
      Serial.print(t);
      Serial.print(", ");
      Serial.println(c);
      
      if (strcmp(t, "retrieve") == 0) {
        updateRobotStatus(true);
      }
      else if (strcmp(t, "frontLed") == 0) {
        setFrontLed(c);
      }
      else if (strcmp(t, "motion") == 0) {
        move(c);
      }
      break;

    case WStype_BIN:
      Serial.print("[WebSocket] Received power value: ");
      power = payload[0];
      Serial.println(power);
      setEdfPower(power);
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
  delay(1000);

  ledInit();

  setRearLed(255, 0, 0); // Status Light: Red

  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect. Needed for native USB port only

  Serial.flush();
  delay(1000);

  escInit();

  motorInit();

  connectWifi();

  setRearLed(255, 255, 0); // Status Light: Yellow

  webSocket.begin("192.168.1.102", 88);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);

  delay(1000);
}

void loop() {
  // Listen for incoming WebSocket messages
  webSocket.loop();
  // Update robot status upon changes
  updateRobotStatus(false);
}

void sendMessage(String type, String content) {
  String json = "{\"type\":\"" + type + "\",\"content\":\"" + content + "\"}";
  webSocket.sendTXT(json);
  Serial.print("[WebSocket] Sent message: ");
  Serial.print(type);
  Serial.print(", ");
  Serial.println(content);
}

void updateRobotStatus(bool reload) {
  if ((reload || connectionTracker.hasChanged()) && connectionTracker.getCurrent()) {
    sendMessage("connection", "connected");
    connectionTracker.setPrevious(connectionTracker.getCurrent());
  }

  if (reload || frontLedTracker.hasChanged()) {
    sendMessage("frontLed", frontLedTracker.getCurrent() ? "on" : "off");
    frontLedTracker.setPrevious(frontLedTracker.getCurrent());
  }

  if (reload || edfPowerTracker.hasChanged()) {
    sendMessage("edfPower", String(edfPowerTracker.getCurrent()));
    edfPowerTracker.setPrevious(edfPowerTracker.getCurrent());
  }

  if (reload || motionTracker.hasChanged()) {
    if (motionTracker.getCurrent() == 0) {
      sendMessage("motion", "stop");
    }
    else if (motionTracker.getCurrent() == 1) {
      sendMessage("motion", "forward");
    }
    else if (motionTracker.getCurrent() == 2) {
      sendMessage("motion", "backward");
    }
    else if (motionTracker.getCurrent() == 3) {
      sendMessage("motion", "left");
    }
    else if (motionTracker.getCurrent() == 4) {
      sendMessage("motion", "right");
    }
    motionTracker.setPrevious(motionTracker.getCurrent());
  }
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

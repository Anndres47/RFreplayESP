
#include <RF433.h>
#include "FS.h"
#include "SPIFFS.h"

#include <WiFi.h>
#include "SinricPro.h"
#include "SinricProLock.h"
#include "SinricProGarageDoor.h"

RF433 rf433(-1, 6);   // rx_pin, tx_pin

bool door_st = false;

void setupWiFi(){
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin("WIFI_NAME", "WIFI_PASSWORD");   // Enter your WIFI credentials

  while (WiFi.status() != WL_CONNECTED){
    Serial.printf(".");
    delay(250);
  }
  //digitalWrite(wifiLed, HIGH);
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

// Door state definitions
#define DOOR_MOVING       0  // Door is not in close and not in open position...somewere between, might be moving
#define DOOR_OPEN_CLOSED  1  // Door is in open/closed position
//#define DOOR_CLOSED       2  // Door is in closed position
#define DOOR_MALFUNCTION  3  // Malfunction! Door can't be open and closed at the same time

bool idle;

int getDoorState() {
  if (!idle) return DOOR_MOVING;
  if (idle)  return DOOR_OPEN_CLOSED;
  return DOOR_MALFUNCTION;
}

bool onLockState(String deviceId, bool &LockState) {
  int doorState = getDoorState();
  bool success = false;
  if (doorState == DOOR_MALFUNCTION){
    Serial.printf("Malfunction! Door is reporting to be open and closed at the same time!\r\n");
    SinricPro.setResponseMessage("Error: malfunction!");
    return false;
  }
  if(doorState == DOOR_OPEN_CLOSED){
    Serial.println("\n\nStarting Send...");
    int result = rf433.sendSignal("test");
    Serial.print("Complete, return code: ");
    Serial.println(result);
    delay(1800);
    if(result == 0){
      idle = false;
      delay(5000);
      idle = true;
      success = true;
    }
  }
  if (!success) SinricPro.setResponseMessage("Error!");
  return success;
}

void setupSinricPro(){
  const char *deviceId = "xxxxxxxxxxxxxxxxxxxxxxxx";  // Should look like "5dc1564130xxxxxxxxxxxxxx"
  SinricProLock &Gate = SinricPro[deviceId];
  Gate.onLockState(onLockState);

  SinricPro.restoreDeviceStates(false);
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });

  // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx","5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
  SinricPro.begin("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx", "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx-xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx");
}

void setup() {
  Serial.begin(115200);

  idle = true;
  
  setupWiFi();
  setupSinricPro();

  Serial.print("\nSetup file system:");
  Serial.println(SPIFFS.begin(true));
  
  rf433.setup();

  //pinMode(1, INPUT_PULLDOWN);
}

bool handleGate(){
  const char *deviceId = "xxxxxxxxxxxxxxxxxxxxxxxx";  // Should look like "5dc1564130xxxxxxxxxxxxxx"
  SinricProGarageDoor &Gate = SinricPro[deviceId];

  if(true){
    Serial.println("\n\nStarting Send...");
    int result = rf433.sendSignal("test");
    Serial.print("Complete, return code: ");
    Serial.println(result);
    if(result == 0){
      door_st = !door_st;
      Gate.sendDoorStateEvent(!door_st);
    }
    delay(2000);
  }
}

void loop() {
  SinricPro.handle();
  //handleGate();
}
/*
  Distance.cpp - Library for using HC-SR04 distance sensor
*/

#include "Arduino.h"
#include "SketchingServer.h"
#include <WiFi.h>
#include <vector>

RestApiClient _restClient;

SketchingServer::SketchingServer(char* ssid, char* pass) {
  _ssid = ssid ? ssid : "sketching";
  _pass = pass ? pass : "with_hardware";
}

void SketchingServer::setup() {
  Serial.println("==========BEGIN WIFI SETUP===========");
  WiFi.mode(WIFI_STA);

  while (1) {
    WiFi.disconnect();
    delay(1000);
    WiFi.begin(_ssid, _pass);
    Serial.print("Connecting to WiFi ");
    Serial.print(_ssid);
    Serial.print(": ");
    while (WiFi.status() == WL_DISCONNECTED) {
      Serial.print('.');
      SketchingServer::wait();
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) break;
  }

  SketchingServer::printStatus();
  Serial.println("========== END WIFI SETUP ===========");
}

Log SketchingServer::log(char *message) {
  return _restClient.log(message);
}

std::vector<Log> SketchingServer::getLogs() {
  return _restClient.getLogs();
}

std::vector<Command> SketchingServer::receiveCommands() {
  std::vector<Command> command;
  while (1) {
    command = _restClient.receiveCommands();
    if (command.size() > 0) {
      return command;
    }

    delay(2000);
  }
}

// PRIVATE

void SketchingServer::printStatus() {
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway address: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("Network strength: ");
  Serial.println(WiFi.RSSI());
}

void SketchingServer::wait() {
  int LED_BUILTIN = 2;
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < 4; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
  }
}

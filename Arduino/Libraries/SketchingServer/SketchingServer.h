/*
  Network.h - Library for Network access
  Expects to run on an ESP32
*/
#ifndef SketchingServer_h
#define SketchingServer_h 

#include "Arduino.h"
#include "RestApiClient.h"

class SketchingServer {
  public:
    SketchingServer(char *ssid, char *pass);
    void setup();
    Log log(char *message);
    std::vector<Log> getLogs();
    std::vector<Command> receiveCommands();
  private:
    void printStatus();
    void wait();
    const char* _ssid;
    const char* _pass;
    RestApiClient _restClient;
};

#endif

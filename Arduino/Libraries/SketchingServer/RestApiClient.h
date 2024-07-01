/*
  RestApiClient.h - Connects to the backend
  Expects to run on an ESP32
*/
#ifndef RestApiClient_h
#define RestApiClient_h 

#include "Arduino.h"
#include <vector>
#include <ArduinoJson.h>

struct Log {
  long id;
  const char *created_at;
  const char *log_message;
};

struct Command {
  long id;
  const char *created_at;
  const char *car_command;
  const char *received;
};

class RestApiClient {
  public:
    RestApiClient();
    Log log(char *message);
    std::vector<Log> getLogs();
    std::vector<Command> receiveCommands();
  private:
    std::vector<char> readResponse();
    bool checkResponse(std::vector<char> response, char *expectedCode);
    const char* getJsonString(std::vector<char> response);
    std::vector<Log> responseToLogs(std::vector<char> response);
    Log responseToLog(std::vector<char> response);
    std::vector<char> getRequest(char *param);
    std::vector<char> postRequest(char *param, JsonDocument doc);
    std::vector<Command> responseToCommands(std::vector<char> response);
};

#endif

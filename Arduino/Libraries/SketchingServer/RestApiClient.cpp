#include "Arduino.h"
#include <WiFi.h>
#include "RestApiClient.h"
#include <vector>
#include <cstring>

WiFiClient wifiClient;

RestApiClient::RestApiClient() {}

std::vector<char> RestApiClient::getRequest(char *param) {
    String protocolLine = "GET ";
    protocolLine += param;
    protocolLine += " HTTP/1.1";

    if (wifiClient.connect("http://sketching.cabbagesandkings.eu", 80)) {

        wifiClient.println(protocolLine);
        wifiClient.println("Host: sketching.cabbagesandkings.eu");
        wifiClient.println();
        wifiClient.flush();
            
        return readResponse();
    }
}

std::vector<char> RestApiClient::postRequest(char *param, JsonDocument doc) {
    String jsonString;
    serializeJson(doc, jsonString);

    String protocolLine = "POST ";
    protocolLine += param;
    protocolLine += " HTTP/1.1";

    if (wifiClient.connect("http://sketching.cabbagesandkings.eu", 80)) {
        wifiClient.println(protocolLine);
        wifiClient.println("Host: sketching.cabbagesandkings.eu");
        wifiClient.println("Content-Type: application/json");
        wifiClient.print("Content-Length: ");
        wifiClient.println(jsonString.length());
        wifiClient.println();
        wifiClient.println(jsonString);
        wifiClient.flush();

        return readResponse();
    } else {
        Serial.println("No connection");
    }
}

Log RestApiClient::log(char *message) {
    JsonDocument doc;
    doc["log_message"] = message;

    std::vector<char> response = postRequest("/log", doc);
    return responseToLog(response);
}

std::vector<Log> RestApiClient::getLogs() {
    std::vector<char> response = getRequest("/log?last=20");
    return responseToLogs(response);
}

std::vector<char> RestApiClient::readResponse() {
    
    while (wifiClient.available() == 0) {};

    std::vector<char> response;
    int readByte = wifiClient.read();
    while (readByte != -1) {
        response.push_back((char) readByte);
        readByte = wifiClient.read();
    }
    return response;
}

std::vector<Command> RestApiClient::receiveCommands() {
    std::vector<char> apiResponse = getRequest("/command");
    return responseToCommands(apiResponse);
}

Log RestApiClient::responseToLog(std::vector<char> response) {
    const char *json = getJsonString(response);
    JsonDocument doc;
    deserializeJson(doc, json);
    free((void*)json);
    const char* log_message = doc["log_message"];
    const char* created_at = doc["created_at"];
    const long id = doc["id"];
    Log log = Log();
    log.created_at = created_at;
    log.log_message = log_message;
    log.id = id;
    return log;
}

std::vector<Log> RestApiClient::responseToLogs(std::vector<char> response) {
    const char *json = getJsonString(response);
    JsonDocument doc;
    deserializeJson(doc, json);
    free((void*)json);
    std::vector<Log> logs;
    for (int i = 0; i < doc.size(); i++) {
       JsonObject json_ob = doc[i];
       const char* log_message = json_ob["log_message"];
       const char* created_at = json_ob["created_at"];
       const long id = json_ob["id"];
       Log log = Log();
       log.created_at = created_at;
       log.log_message = log_message;
       log.id = id;
       logs.push_back(log);
    }
    return logs;
}

std::vector<Command> RestApiClient::responseToCommands(std::vector<char> response) {
    const char *json = getJsonString(response);
    JsonDocument doc;
    deserializeJson(doc, json);
    free((void*)json);
    std::vector<Command> commands;
    for (int i = 0; i < doc.size(); i++) {
       JsonObject json_ob = doc[i];
       const char* car_command = json_ob["car_command"];
       const char* created_at = json_ob["created_at"];
       const char* received = json_ob["received"];
       const long id = json_ob["id"];
       Command command = Command();
       command.created_at = created_at;
       command.car_command = car_command;
       command.received = received;
       command.id = id;
       commands.push_back(command);
    }
    return commands;
}

bool RestApiClient::checkResponse(std::vector<char> response, char *expectedCode) {
    char httpCode[4];
    httpCode[3] = '\0';
    for (int i = 9; i < 12; i++) {
        httpCode[i-9] = response[i];
    }

    return std::strcmp(httpCode, expectedCode) == 0;
}

// Hands back ownership
const char* RestApiClient::getJsonString(std::vector<char> response) {
    int count = 0;
    int begin;
    int size = response.size();
    for (int i = 0; i < size -1; i++) {
        if (response[i] == '\r' && response[i+1] == '\n' && response[i+2] == '\r' && response[i+3] == '\n') {
            begin = i + 3;
            break;
        }
    }


    char *json = (char*) malloc(sizeof(char) * (size - begin + 1));
    int i = 0;
    while (i < size - begin) {
        json[i] = response[i + begin];
        i++;
    }
    json[i] = '\0';
    return json;
}

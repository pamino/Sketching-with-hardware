# Network Library

## Example implementation
When no SSID and password are provided, we default to SSID "sketching" and password "with_hardware".

Network.setup() blocks until it has found a wifi network
network.receiveCommands blocks until the server had some commands. 
The server marks those commands automatically as read as soon as they were 
downloaded. Commands come as vector. We go through the vector in a for loop 
and execute all commands. Afterwards, it looks for new commands.

If wifi breaks off, we can only reset the device via button. 
There is no recovering from it.

Ran out of time, so almost no error checking, hence the no recovery and need
for being reset.

```C
#include <SketchingServer.h>

// SketchingServer network(SSID, PASSWORD);
SketchingServer network(NULL, NULL);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("\nInitialising: ");
  network.setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  std::vector<Command> commands = network.receiveCommands();
  for (int command_iterator = 0; command_iterator < commands.size(); command_iterator++) {
    Command command = commands[command_iterator];
    Serial.print("Running command '");
    Serial.print(command.car_command);
    Serial.println("'");

    if (strcmp("print_log", command.car_command) == 0) {
        std::vector<Log> logs = network.getLogs();
        for (int i = 0; i < logs.size(); i++) {
          Serial.print(logs[i].created_at);
          Serial.print(": ");
          Serial.println(logs[i].log_message);
        }
    } else if (strcmp("test_log", command.car_command) == 0) {
        Log log = network.log("New Testlog");
        Serial.print(log.created_at);
        Serial.print(": ");
        Serial.println(log.log_message);
    }
  }
}
```

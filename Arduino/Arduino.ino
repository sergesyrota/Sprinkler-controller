#include <EEPROMex.h>
#include <SyrotaAutomation1.h>
#include <DHT.h>
#include "printf.h"
#include "config.h"

//
// Hardware configuration
//

SyrotaAutomation net = SyrotaAutomation(RS485_CONTROL_PIN);
DHT dht;

char buf[100];
dht_t dhtData;

struct configuration_t conf = {
  CONFIG_VERSION,
  // Default values for config
  9600UL, //unsigned long baudRate; // Serial/RS-485 rate: 9600, 14400, 19200, 28800, 38400, 57600, or 115200
};

struct valve_t valve[2] = {
  {VALVE0_PIN, LOW, 0, 0},
  {VALVE1_PIN, LOW, 0, 0}
};

void setup(void)
{
  readConfig();
  // Set device ID
  strcpy(net.deviceID, NET_ADDRESS);
  Serial.begin(conf.baudRate);

  dht.setup(DHT11_PIN, DHT::DHT11);
  updateDht();
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
}

void readConfig()
{
  // Check to make sure config values are real, by looking at first 3 bytes
  if (EEPROM.read(0) == CONFIG_VERSION[0] &&
    EEPROM.read(1) == CONFIG_VERSION[1] &&
    EEPROM.read(2) == CONFIG_VERSION[2]) {
    EEPROM.readBlock(0, conf);
  } else {
    // Configuration is invalid, so let's write default to memory
    saveConfig();
  }
}

void saveConfig()
{
  EEPROM.writeBlock(0, conf);
}

void loop(void)
{
  // Process RS-485 commands
  if (net.messageReceived()) {
    if (net.assertCommandStarts("getDht", buf)) {
      sprintf(buf, "%dC; %d%%RH; %ds ago", dhtData.temperature, dhtData.humidity, (millis() - dhtData.lastSuccessTime)/1000);
      net.sendResponse(buf);
    } else if (net.assertCommandStarts("set", buf)) {
      processSetCommands();
    } else if (net.assertCommandStarts("openValve0:", buf)) {
      // Number signifies seconds to keep valve open
      unsigned long tmp = strtol(buf, NULL, 10);
      openValve(0, tmp);
      net.sendResponse("OK");
    } else if (net.assertCommandStarts("openValve1:", buf)) {
      // Number signifies seconds to keep valve open
      unsigned long tmp = strtol(buf, NULL, 10);
      openValve(1, tmp);
      net.sendResponse("OK");
    } else if (net.assertCommandStarts("statusValve:", buf)) {
      unsigned long tmp = strtol(buf, NULL, 10);
      statusValve(tmp);
    } else if (net.assertCommandStarts("closeValve:", buf)) {
      unsigned long tmp = strtol(buf, NULL, 10);
      closeValve(tmp);
      net.sendResponse("OK");
    } else {
      net.sendResponse("Unrecognized command");
    }
  }
  
  // Check DHT sensor
  if (millis() - dhtData.lastAttemptTime > DHT_UPDATE_INTERVAL) {
    updateDht();
  }
  
  // Check if any valves need to be closed
  checkValves();
}

void statusValve(int i) {
  if (valve[i].status == HIGH) {
    unsigned long millisLeft = valve[i].openMillisDesired - (millis() - valve[i].timeOpened);
    sprintf(buf, "OPENED; %lus left", millisLeft/1000);
  } else {
    sprintf(buf, "CLOSED");
  }
  net.sendResponse(buf);
}

void checkValves()
{
  for (int i=0; i<2; i++)
  {
    if (HIGH == valve[i].status && 
      // All operations with millis may include rollover, so has to be done using substraction only
      (millis() - valve[i].timeOpened > valve[i].openMillisDesired))
    {
      closeValve(i);
    }
    // Just in case digital write current known status anyways
    digitalWrite(valve[i].pin, valve[i].status);
  }
}

void closeValve(int id)
{
  digitalWrite(valve[id].pin, LOW);
  valve[id].status = LOW;
}

void openValve(int id, unsigned long time)
{
  digitalWrite(valve[id].pin, HIGH);
  valve[id].status = HIGH;
  valve[id].timeOpened = millis();
  valve[id].openMillisDesired = time*1000UL;
}

void updateDht()
{
  // Throw out result to make sure we don't have an error
  dht.getTemperature();
  dhtData.lastAttemptTime = millis();
  if (dht.getStatus() == dht.ERROR_NONE) {
    dhtData.lastSuccessTime = millis();
    dhtData.temperature = dht.getTemperature();
    dhtData.humidity = dht.getHumidity();
  }
}

// Write to the configuration when we receive new parameters
void processSetCommands()
{
  if (net.assertCommandStarts("setBaudRate:", buf)) {
    long tmp = strtol(buf, NULL, 10);
    // Supported: 9600, 14400, 19200, 28800, 38400, 57600, or 115200
    if (tmp == 9600 ||
      tmp == 14400 ||
      tmp == 19200 ||
      tmp == 28800 ||
      tmp == 38400 ||
      tmp == 57600 ||
      tmp == 115200
    ) {
      conf.baudRate = tmp;
      saveConfig();
      net.sendResponse("OK");
      Serial.end();
      Serial.begin(tmp);
    } else {
      net.sendResponse("ERROR");
    }
  } else {
    net.sendResponse("Unrecognized");
  }
}



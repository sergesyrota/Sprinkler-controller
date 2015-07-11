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
    } else {
      net.sendResponse("Unrecognized command");
    }
  }
  
  // Check DHT sensor
  if (millis() - dhtData.lastAttemptTime > DHT_UPDATE_INTERVAL) {
    updateDht();
  }
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



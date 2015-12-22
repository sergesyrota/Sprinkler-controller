// Debug mode. Comment out to disable.
//#define DEBUG

// Config version
#define CONFIG_VERSION "SP1"

// SyrotaAutomation parameters
#define RS485_CONTROL_PIN 2
#define NET_ADDRESS "Sprinkler1"

#define DHT11_PIN 10
#define DHT_UPDATE_INTERVAL 60000
#define VALVE0_PIN 4
#define VALVE1_PIN 5

struct configuration_t {
  char checkVersion[4]; // This is for detection if we have right settings or not
  unsigned long baudRate; // Serial/RS-485 rate: 9600, 14400, 19200, 28800, 38400, 57600, or 115200
};

struct dht_t {
  unsigned long lastAttemptTime;
  unsigned long lastSuccessTime;
  int temperature;
  int humidity;
};

struct valve_t {
  int pin;
  int status;
  unsigned long timeOpened;
  unsigned long openMillisDesired;
};

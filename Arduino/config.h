// Debug mode. Comment out to disable.
//#define DEBUG

// Config version
#define CONFIG_VERSION "SP2"

// SyrotaAutomation parameters
#define RS485_CONTROL_PIN 2
#define NET_ADDRESS "Sprinkler1"

#define DHT11_PIN 10
#define DHT_UPDATE_INTERVAL 60000
#define VALVE0_PIN 4
#define VALVE1_PIN 5
#define LEAK_UPDATE_INTERVAL 30000
#define LEAK_ENABLE_PIN 3
#define LEAK_SENSOR_PIN A3
#define BUZZER_PIN 6

struct configuration_t {
  char checkVersion[4]; // This is for detection if we have right settings or not
  unsigned long baudRate; // Serial/RS-485 rate: 9600, 14400, 19200, 28800, 38400, 57600, or 
  unsigned int leakThreshold; // threshold after which sensor considers leak is detected
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

struct leak_t {
  unsigned long lastSuccessTime;
  int value;
  boolean alarm;
  boolean silenceAlarm;
  unsigned long lastBuzzerEnabled;
};

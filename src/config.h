#define MQTT_SERVER_STRLEN 64
#define MQTT_PORT_BYTES_LEN 4
#define MQTT_USERNAME_STRLEN 64
#define MQTT_PASSWORD_STRLEN 64
#define CONFIG_LEN 196

#ifndef CONFIG_DATA_STRUCT
#define CONFIG_DATA_STRUCT

struct configDataStruct { 
  char mqttServer[MQTT_SERVER_STRLEN] = "";
  unsigned long mqttPort = 1883;
  char mqttUsername[MQTT_USERNAME_STRLEN] = "";
  char mqttPassword[MQTT_PASSWORD_STRLEN] = "";
};

#endif

void setUpConfig();
void saveConfigCallback();
char* getDeviceName();
char* getMqttServer();
unsigned long getMqttPort();
char* getMqttUsername();
char* getMqttPassword();
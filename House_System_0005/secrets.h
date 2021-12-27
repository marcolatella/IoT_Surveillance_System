// Use this file to store all of the private credentials and connection details

// MQTT access
#define MQTT_BROKERIP ""                              // IP address of the machine running the MQTT broker
#define MQTT_CLIENTID ""                                    // client identifier
#define MQTT_USERNAME ""                                   // mqtt user's name
#define MQTT_PASSWORD ""                                    // mqtt user's password

// WiFi configuration
//#define SECRET_SSID ""                                   // SSID
//#define SECRET_PASS ""                                       // WiFi password

// WiFi configuration outdoor
#define SECRET_SSID ""                                // SSID
#define SECRET_PASS ""                                     // WiFi password

// MySQL access
#define MYSQL_IP {}                                   // IP address of the machine running MySQL
#define MYSQL_USER ""                                        // db user
#define MYSQL_PASS ""                                       // db user's password

// OpenWeathermap.org configuration
#define WEATHER_API_KEY ""           // api key form https://home.openweathermap.org/api_keys
#define WEATHER_CITY "Milan"                                         // city
#define WEATHER_COUNTRY "it"                                         // ISO3166 country code 

// Telegram bot configuration (data obtained form @BotFather, see https://core.telegram.org/bots#6-botfather)
#define BOT_NAME ""                           // name displayed in contact list
#define BOT_USERNAME ""                                  // short bot id
#define BOT_TOKEN ""   // authorization token

// ONLY if static configuration is needed
/*
#define IP {192, 168, 1, 100}                    // IP address
#define SUBNET {255, 255, 255, 0}                // Subnet mask
#define DNS {149, 132, 2, 3}                     // DNS
#define GATEWAY {149, 132, 182, 1}               // Gateway
*/

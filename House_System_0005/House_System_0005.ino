/* BEGIN Includes ------------------------------------------------------------- */ 
#include <ESP8266TelegramBOT.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
// MySQL libraries
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "secrets.h"

/* END Includes ------------------------------------------------------------- */

// WiFi cfg
#ifdef IP
IPAddress ip(IP);
IPAddress subnet(SUBNET);
IPAddress dns(DNS);
IPAddress gateway(GATEWAY);
#endif

// MySQL client
WiFiClient dbClient;
// MQTT client
WiFiClient networkClient;
// OpenWeather client
WiFiClient weatherClient;

// WiFi Credentials
char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password

// MySQL server cfg
char mysql_user[] = MYSQL_USER;       // MySQL user login username
char mysql_password[] = MYSQL_PASS;   // MySQL user login password
IPAddress server_addr(MYSQL_IP);      // IP of the MySQL *server* here
MySQL_Connection conn((Client *)&dbClient);

char query[512];
char INSERT_DATA[] = "INSERT INTO `mlatella`.`systems_data` (`mac`, `user_id`,`active_contract`,`connected_system`,`armed`,`alarmed`) VALUES ('%s',%s,%d,%d,%d,%d)";

// CLIENT_ID data
String CLIENT_ID = "0005";


// PIR Sensor
#define PIR_SENSOR D8               // input pin for PIR Sensor
int val = 0;                        // variable for reading the PIN_PIR status
static unsigned long temp = 0;      // variable to time the PIR reading

// RFID 
#define RST_PIN D3     
#define SS_PIN D2
MFRC522 rfid(SS_PIN, RST_PIN);      // Instance of the class

// LED
#define EXTERNAL_LED LED_BUILTIN
static unsigned long led_t = 0;
static unsigned long db_t = 0;
static bool led_stat = true;
static bool connected_system = false;

// BUTTONS
#define BUTTON_DEBOUNCE_DELAY 20
#define SLEEP_BUTTON D1

// Telegram bot
TelegramBOT bot(BOT_TOKEN, BOT_NAME, BOT_USERNAME);
#define BOT_MTBS 5000                   // mean time between scans for new messages
static unsigned long botLastScanTime;   // last time messages' scan has been done

// System variable
static String MAC_ADDR = WiFi.macAddress();
static bool active_contract = true;
static bool to_initialize = true;
static bool reset_system;
static bool armed = false;
static bool alarmed = false;
static byte setted_key[4];
static byte coming_key[4];
static bool sleep_mode = false;
static unsigned long weather_t = 0;
static String last_weather = "";
static String new_weather = "";
static String weather_descr = "";
static bool bot_activated = false;
static String chat_id = "";
static bool notify = true;
static byte sleep_count = 0;
static bool notify_armed = true;

// MQTT data
MQTTClient mqttClient;                                                           // handles the MQTT communication protocol
#define MQTT_TOPIC_CONFIGURATION "surv_sys/hs/config"                            // output topic to initialise and add the house alarm to the system
String MQTT_TOPIC_ARMED = "surv_sys/hs/" + MAC_ADDR + "/armed";                  // output topic to comunicate the changing alarm status
String MQTT_TOPIC_ALARMED = "surv_sys/hs/" + MAC_ADDR + "/alarmed";              // output topic to comunicate the motion detected
String MQTT_TOPIC_CONNECT = "surv_sys/hs/" + MAC_ADDR + "/connect";              // output topic to comunicate the occured connection to the broker
String MQTT_TOPIC_LW = "surv_sys/hs/" + MAC_ADDR + "/last_will";                 // output topic to comunicate the last will                                     
String MQTT_TOPIC_CONTROL = "surv_sys/hs/" + MAC_ADDR + "/control";              // input topic to control arm/disarm the system
String MQTT_TOPIC_CONTRACT = "surv_sys/hs/" + MAC_ADDR + "/contract";            // input topic to set the state of the contract                                    
String MQTT_TOPIC_RESET_RFID = "surv_sys/hs/" + MAC_ADDR + "/reset_rfid";        // input topic to reset the card configuration 

// weather api (refer to https://openweathermap.org/current)
const char weather_server[] = "api.openweathermap.org";
const char weather_query[] = "GET /data/2.5/weather?q=%s,%s&units=metric&APPID=%s";

void setup() {
  WiFi.mode(WIFI_STA);

  // PIN configuration
  pinMode(PIR_SENSOR, INPUT);
  pinMode(EXTERNAL_LED, OUTPUT);
  digitalWrite(EXTERNAL_LED, led_stat);
  pinMode(SLEEP_BUTTON, INPUT_PULLUP);
  pinMode(D0, WAKEUP_PULLUP);
  
  // setup MQTT
  sendLastWill();
  mqttClient.begin(MQTT_BROKERIP, 1883, networkClient);   // setup communication with MQTT broker
  mqttClient.onMessage(mqttMessageReceived);              // callback on message received from MQTT broker

  // EEPROM Configuration
  EEPROM.begin(2048); //Size can be anywhere between 4 and 4096 bytes.

  Serial.begin(115200);
  Serial.println(F("\n\nSetup completed.\n\n"));

  // Read the information saved in the EEPROM
  readEEPROM();
  int p = EEPROM_read(3, setted_key);
  bot_activated = EEPROM.read(8);
  if (bot_activated) {
      int i = readStringFromEEPROM(9, &chat_id);
  }

  // Control prints
  Serial.println(active_contract);
  Serial.println(to_initialize);
  Serial.println(sleep_mode);
  Serial.println(setted_key[0], HEX);
  Serial.println(setted_key[1], HEX);
  Serial.println(setted_key[2], HEX);
  Serial.println(setted_key[3], HEX);
  Serial.println(bot_activated);
  Serial.println(chat_id);

  // RFID setup
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
}

void loop() {

  /**
   * Sleep mode management
   */
  if(sleep_mode && digitalRead(SLEEP_BUTTON)){
    digitalWrite(EXTERNAL_LED, LOW);
    sleep_count = EEPROM.read(19);
    sleep_count++;
    Serial.print("Sleep count: ");
    Serial.println(sleep_count);
    if(sleep_count >= 40){
      sleep_count = 0;
      connectToWiFi();
      WriteMultiToDB(MAC_ADDR, CLIENT_ID, active_contract, false, armed, alarmed);
    }
    EEPROM.write(19, sleep_count);
    EEPROM.commit();
    Serial.println("Going to sleep...");
    digitalWrite(EXTERNAL_LED, HIGH);
    ESP.deepSleep(15e5);
  }
  sleep_mode = false;
  
  connectToWiFi();             // connect to WiFi (if not already connected)
  connectToMQTTBroker();       // connect to MQTT broker (if not already connected)
  mqttClient.loop();           // MQTT client loop
  
  if(!active_contract) {
    return;
  }

  /**
   * Initial RFID card configuration
   */
  if(to_initialize){
    if (millis() - led_t > 900){
      led_stat = !led_stat;
      digitalWrite(EXTERNAL_LED, led_stat);
      led_t = millis();
    }
    configKeyCard();
    return;
  }

  /**
   * Function that reads RFID Cards
   */
  readKeyCard();

  /**
   * IF statement that manage the ARMED/DISARMED Telegram notification
   */
  if(notify_armed){
    String msg = "=== System " + String(armed == true ? "ARMED" : "DISARMED") + "! ===%0A%0ALocation: " + String(WEATHER_CITY) + "%0A" +
                     "User: User_" + CLIENT_ID + "%0A" +
                     "=========================";
    bot.sendMessage(chat_id, msg, "");
    notify_armed = false;
  }

  /**
   * IF statement that manage the ARMED/DISARMED/ALARMED status
   */
  if(armed){
    if (millis() - temp > 200){
      if (!alarmed) { // if system is alarmed we can skip to control PIR
        Serial.println("Armed");
        controlPIR();
      } else {
        if(notify && bot_activated){
          String msg = "=== INTRUSION DETECTED! ===%0A%0ALocation: " + String(WEATHER_CITY) + "%0A" +
                       "User: User_" + CLIENT_ID + "%0A" +
                       "=========================";
          bot.sendMessage(chat_id, msg, "");
          notify = false;
        }
        if (millis() - led_t > 300) {
          Serial.println("SYSTEM ALARMED!");
          led_stat = !led_stat;
          digitalWrite(EXTERNAL_LED, led_stat);
          led_t = millis();
        }
      }
      temp = millis();
    }
  } else {
    if (millis() - temp > 5000){
      digitalWrite(EXTERNAL_LED, LOW);
      Serial.println("Disarmed");
      temp = millis();
      delay(20);
      digitalWrite(EXTERNAL_LED, HIGH);
    }
  }

  /**
   * 30sec Timer to write info on MySQL
   */
  if (millis() - db_t > 30000){
    WriteMultiToDB(MAC_ADDR, CLIENT_ID, active_contract, connected_system, armed, alarmed);
    db_t = millis();
  }

  /**
   * 30min Timer to check weather and notify user via Telegram
   */
  if ((millis() - weather_t > 5000) && armed && !alarmed){
    currentWeather();
    if(bot_activated){
      if (new_weather.equals("Thunderstorm") && !new_weather.equals(last_weather)) {
          last_weather = new_weather;
          String msg = "===== Weather Alert! =====%0APossible malfunctions with the alarm system!%0A%0ALocation: " + String(WEATHER_CITY) + "%0A" +
                       "Weather: " + new_weather + "%0A" +
                       "Weather description: " + weather_descr + "%0A" +
                       "======================";
          bot.sendMessage(chat_id, msg, "");
      }
    }
    weather_t = millis();
  }

  /**
   * IF statement to get message from Telegram bot
   */
  if ((millis() - botLastScanTime > BOT_MTBS)) {
    bot.getUpdates(bot.message[0][1]);   // launch API GetUpdates up to xxx message
    botExecMessages();                   // process received messages
    botLastScanTime = millis();
  }

  /**
   * IF statement to turn on DeepSleep mode
   */
  if (isButtonPressed() == true){
    Serial.println("Button Pressed");
    Serial.println("Saving data in EPROM...");
    sleep_mode = true;
    writeEEPROM(active_contract, to_initialize, sleep_mode);
    int p = EEPROM_write(3, setted_key);
    Serial.println("Going to sleep...");
    ESP.deepSleep(5e6);
  }
}

/**
 * Function that print Wifi Status info
 */
void printWifiStatus() {
  // print the SSID of the network you're attached to
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print your board's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print(F("Signal strength (RSSI):"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
  Serial.println(WiFi.macAddress());
}


/**
 * Function to connect to WiFI
 */
void connectToWiFi(){
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("Connecting to SSID: "));
    Serial.println(SECRET_SSID);
    connected_system = false;

  #ifdef IP
    WiFi.config(ip, dns, gateway, subnet);   // by default network is configured using DHCP
  #endif

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(F("."));
      delay(250);
    }
    Serial.println(F("\nConnected!"));
    printWifiStatus();
  }

}

/**
 * Function to connect to MQTT Broker
 */
void connectToMQTTBroker() {
  if (!mqttClient.connected()) {   // not connected
    Serial.print(F("\nConnecting to MQTT broker..."));
    while (!mqttClient.connect(MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.print(F("."));
      delay(1000);
    }
    Serial.println(F("\nConnected!"));

    // connected to broker, subscribe topics
    mqttClient.subscribe(MQTT_TOPIC_CONTROL);
    Serial.println(F("\nSubscribed to Control topic!"));
    mqttClient.subscribe(MQTT_TOPIC_CONTRACT);
    Serial.println(F("\nSubscribed to Contract topic!"));
    mqttClient.subscribe(MQTT_TOPIC_RESET_RFID);
    Serial.println(F("\nSubscribed to Reset topic!"));
    if(!connected_system){
        sendMQTTConnection();
        connected_system = true;
    }
  }
}

/**
 * Function to config new RFID card
 */
void configKeyCard() {
  if (rfid.PICC_IsNewCardPresent()){
    // Verify if the NUID has been readed
    if (rfid.PICC_ReadCardSerial()){
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.println(rfid.PICC_GetTypeName(piccType));
      
      // Check is the PICC of Classic MIFARE type
      if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("Your tag is not of type MIFARE Classic."));
        return;
      }
  
      for (byte i = 0; i < 4; i++) {
        setted_key[i] = rfid.uid.uidByte[i];
      }
  
      Serial.println("A new card has been attached:");
      Serial.println(setted_key[0], HEX);
      Serial.println(setted_key[1], HEX);
      Serial.println(setted_key[2], HEX);
      Serial.println(setted_key[3], HEX);
      EEPROM_write(3, setted_key);
      
      // Halt PICC
      rfid.PICC_HaltA();
  
      // Stop encryption on PCD
      rfid.PCD_StopCrypto1();
  
      to_initialize = false;
      led_stat = true;
      digitalWrite(EXTERNAL_LED, led_stat);
      sendMQTTConfig();
    }
  } 
}

/**
 * Function to read RFID cards
 */
void readKeyCard() {
  if (rfid.PICC_IsNewCardPresent()){
    // Verify if the NUID has been readed
    if (rfid.PICC_ReadCardSerial()){
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.println(rfid.PICC_GetTypeName(piccType));
      
      // Check is the PICC of Classic MIFARE type
      if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("Your tag is not of type MIFARE Classic."));
        return;
      }

      if (rfid.uid.uidByte[0] != setted_key[0] || 
          rfid.uid.uidByte[1] != setted_key[1] || 
          rfid.uid.uidByte[2] != setted_key[2] || 
          rfid.uid.uidByte[3] != setted_key[3] ) {
        Serial.println(F("The card is not correct!"));
      }else{
        Serial.println("Correct card!");
        armed = !armed;
        notify_armed = true;
        if (armed) {
          led_stat = false;
          digitalWrite(EXTERNAL_LED, led_stat);
        } else {
          led_stat = true;
          digitalWrite(EXTERNAL_LED, led_stat);
        }
        if(alarmed){
          alarmed = false;
          notify = true;
          sendMQTTAlarmed();
        }
        sendMQTTArmed();
      } 
      // Halt PICC
      rfid.PICC_HaltA();
      // Stop encryption on PCD
      rfid.PCD_StopCrypto1();
    }
  }
}

/**
 * Function to send MQTT config message
 */
void sendMQTTConfig() {
  // publish MQTT configuration (as a JSON object)
  const int capacity = JSON_OBJECT_SIZE(10);
  StaticJsonDocument<capacity> doc;
  doc["userID"] = CLIENT_ID;
  doc["mac"] = MAC_ADDR;
  char buffer[128];
  size_t n = serializeJson(doc, buffer);
  Serial.print(F("JSON message: "));
  Serial.println(buffer);
  mqttClient.publish(MQTT_TOPIC_CONFIGURATION, buffer, n);
}

/**
 * Function to send MQTT armed message
 */
void sendMQTTArmed() {
  // publish MQTT configuration (as a JSON object)
  const int capacity = JSON_OBJECT_SIZE(10);
  StaticJsonDocument<capacity> doc;
  doc["userID"] = CLIENT_ID;
  doc["mac"] = MAC_ADDR;
  if (armed){
    doc["armed"] = "true";
  } else {
    doc["armed"] = "false";
  }
  char buffer[128];
  size_t n = serializeJson(doc, buffer);
  Serial.print(F("JSON message: "));
  Serial.println(buffer);
  mqttClient.publish(MQTT_TOPIC_ARMED.c_str(), buffer, n);
}

/**
 * Function to send MQTT connection message
 */
void sendMQTTConnection() {
  // publish MQTT configuration (as a JSON object)
  const int capacity = JSON_OBJECT_SIZE(10);
  StaticJsonDocument<capacity> doc;
  doc["ID"] = CLIENT_ID;
  doc["m"] = MAC_ADDR;
  doc["c"] = "true";
  char buffer[128];
  size_t n = serializeJson(doc, buffer);
  Serial.print(F("JSON message: "));
  Serial.println(buffer);
  mqttClient.publish(MQTT_TOPIC_CONNECT.c_str(), buffer, n);
}

/**
 * Function to set MQTT last will message
 */
void sendLastWill(){
  const int capacity = JSON_OBJECT_SIZE(10);
  StaticJsonDocument<capacity> doc;
  doc["ID"] = CLIENT_ID;
  doc["m"] = MAC_ADDR;
  char buffer[128];
  size_t n = serializeJson(doc, buffer);
  mqttClient.setWill(MQTT_TOPIC_LW.c_str(), buffer);
}

/**
 * Function to send MQTT alarmed message
 */
void sendMQTTAlarmed() {
  // publish MQTT configuration (as a JSON object)
  const int capacity = JSON_OBJECT_SIZE(10);
  StaticJsonDocument<capacity> doc;
  doc["userID"] = CLIENT_ID;
  doc["mac"] = MAC_ADDR;
  if (alarmed){
    doc["alarmed"] = "true";
  } else {
    doc["alarmed"] = "false";
  }
  char buffer[128];
  size_t n = serializeJson(doc, buffer);
  Serial.print(F("JSON message: "));
  Serial.println(buffer);
  mqttClient.publish(MQTT_TOPIC_ALARMED.c_str(), buffer, n);
}

/**
 * Function to manage the MQTT messages received
 */
void mqttMessageReceived(String &topic, String &payload) {
  // this function handles a message from the MQTT broker
  Serial.println("Incoming MQTT message: " + topic + " - " + payload);

  if (topic == MQTT_TOPIC_CONTROL) {
    // deserialize the JSON object
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload);
    const char *mac_control = doc["mac"];
    
    if (strcmp(mac_control, MAC_ADDR.c_str()) != 0) {       // received payload is not: {"mac": MAC_ADDR}
      return;
    }
    const char *armed_control = doc["armed"];
    
    if (!active_contract) {
      return;
    }
    notify_armed = true;
    if (strcmp(armed_control, "true") == 0) {   
      armed = true;
      led_stat = false;
      digitalWrite(EXTERNAL_LED, led_stat);
      sendMQTTArmed();
    } else if (strcmp(armed_control, "false") == 0) { 
      armed = false;
      led_stat = true;
      digitalWrite(EXTERNAL_LED, led_stat);
      if (alarmed) {
        alarmed = false;
        notify = true;
        sendMQTTAlarmed();
      }
      sendMQTTArmed();
    } else {
      Serial.println(F("MQTT Payload not recognized, message skipped"));
    }
  } else if (topic == MQTT_TOPIC_CONTRACT) {
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload);
    const char *mac_control = doc["mac"];
    
    if (strcmp(mac_control, MAC_ADDR.c_str()) != 0) {       // received payload is not: {"mac": MAC_ADDR}
      return;
    }
    
    const char *contract_control = doc["contract"];
    
    if (strcmp(contract_control, "true") == 0) {   
      active_contract = true;
    } else if (strcmp(contract_control, "false") == 0) { 
      Serial.println("Contract Expired!");
      active_contract = false;
      if (armed) {
        armed = false;
        sendMQTTArmed();
      }
      if (alarmed) {
        alarmed = false;
        sendMQTTAlarmed();
      }
      led_stat = true;
      digitalWrite(EXTERNAL_LED, led_stat);
    }
    
  } else if(topic == MQTT_TOPIC_RESET_RFID) {
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload);
    const char *mac_control = doc["mac"];
    if (strcmp(mac_control, MAC_ADDR.c_str()) != 0) {       // received payload is not: {"mac": MAC_ADDR}
      return;
    }
    const bool reset_system = doc["reset"];
    if (reset_system == true) {   
      to_initialize = true;
    }  
  } else {
    Serial.println(F("MQTT Topic not recognized, message skipped"));
  }
}

/**
 * Function that manage the PIR sensor
 */
void controlPIR() {
  val = digitalRead(PIR_SENSOR);   // read input value
  if (val == HIGH) {               // check if the input is HIGH
    alarmed = true;
    sendMQTTAlarmed();
    WriteMultiToDB(MAC_ADDR, CLIENT_ID, active_contract, connected_system, armed, alarmed);
    db_t = millis();
  } 
}

/**
 * Function that write data on MySQL Database
 */
int WriteMultiToDB(String MAC_ADDR, String CLIENT_ID, bool active_contract, bool connected_system, bool armed, bool alarmed) {
  int error;
  if (conn.connect(server_addr, 3306, mysql_user, mysql_password)) {
    Serial.println(F("MySQL connection established."));

    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

    sprintf(query, INSERT_DATA, MAC_ADDR.c_str(), CLIENT_ID.c_str(), active_contract, connected_system, armed, alarmed);
    Serial.println(query);
    // execute the query
    cur_mem->execute(query);
    // Note: since there are no results, we do not need to read any data
    // deleting the cursor also frees up memory used
    delete cur_mem;
    error = 1;
    Serial.println(F("Data recorded on MySQL"));

    conn.close();
  } else {
    Serial.println(F("MySQL connection failed."));
    error = -1;
  }

  return error;
}

/**
 * Debouncing function for sleep button
 */
boolean isButtonPressed() {
  static byte lastState = digitalRead(SLEEP_BUTTON); // the previous reading from the input pin
  for (byte count = 0; count < BUTTON_DEBOUNCE_DELAY; count++) {
    if (digitalRead(SLEEP_BUTTON) == lastState) return false;
      delay(1);
  }
  
  lastState = !lastState;
  return lastState == HIGH ? false : true;
}

/**
 * Function to Write general data on EEPROM
 */
template <class T> int EEPROM_write(int ee, const T& value){
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++); //does not write to flash immediately, instead you must call EEPROM.commit()
  EEPROM.commit();
  return i;
}

/**
 * Function to Read general data on EEPROM
 */
template <class T> int EEPROM_read(int ee, T& value){
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}

/**
 * Function to Write boolean on EEPROM
 */
void writeEEPROM(bool ac_co, bool to_init, bool sl_md) {
  // write to EEPROM.
  EEPROM.write(0, ac_co);
  EEPROM.commit();    //Store data to EEPROM
  EEPROM.write(1, to_init);
  EEPROM.commit();    //Store data to EEPROM
  EEPROM.write(2, sl_md);
  EEPROM.commit();    //Store data to EEPROM
}

/**
 * Function to Read boolean on EEPROM
 */
bool readEEPROM() {
  bool ac_co = EEPROM.read(0);
  active_contract = ac_co;
  bool to_init = EEPROM.read(1);
  to_initialize = to_init;
  bool sleep = EEPROM.read(2);
  sleep_mode = sleep;
}

/**
 * Function to Write String on EEPROM
 */
int writeStringToEEPROM(int addrOffset, const String &strToWrite) {
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
  return addrOffset + 1 + len;
}

/**
 * Function to Read String on EEPROM
 */
int readStringFromEEPROM(int addrOffset, String *strToRead) {
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0'; 
  *strToRead = String(data);
  return addrOffset + 1 + newStrLen;
}

/**
 * Function to manage weather data 
 */
void currentWeather() {
  // Current weather api documentation at: https://openweathermap.org/current
  Serial.println(F("\n=== Current weather ==="));

  // call API for current weather
  if (weatherClient.connect(weather_server, 80)) {
    char request[100];
    sprintf(request, weather_query, WEATHER_CITY, WEATHER_COUNTRY, WEATHER_API_KEY);
    weatherClient.println(request);
    weatherClient.println(F("Host: api.openweathermap.org"));
    weatherClient.println(F("User-Agent: ArduinoWiFi/1.1"));
    weatherClient.println(F("Connection: close"));
    weatherClient.println();
  } else {
    Serial.println(F("Connection to api.openweathermap.org failed!\n"));
  }

  while (weatherClient.connected() && !weatherClient.available()) delay(1);   // wait for data
  String result;
  while (weatherClient.connected() || weatherClient.available()) {   // read data
    char c = weatherClient.read();
    result = result + c;
  }

  weatherClient.stop();   // end communication

  char jsonArray[result.length() + 1];
  result.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, jsonArray);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  
  new_weather = doc["weather"][0]["main"].as<String>();
  weather_descr = doc["weather"][0]["description"].as<String>();
  Serial.print(F("Weather: "));
  Serial.println(new_weather);


  Serial.println(F("==============================\n"));
}

/**
 * Function to manage message received from telegram via bot
 */
void botExecMessages() {
  for (int i = 1; i < bot.message[0][0].toInt() + 1; i++) {
    String messageRcvd = bot.message[i][5];
    Serial.println(messageRcvd);
    char *msg = const_cast<char*>(messageRcvd.c_str());
    if (strstr(messageRcvd.c_str(), "/config")){  
      String cmd = (String)strtok(msg, ":");
      String user = (String)strtok(NULL, ":");
      String pass = (String)strtok(NULL, ":");
      String user_saved = "User_" + String(CLIENT_ID);
      String pass_saved = "Pass_" + String(CLIENT_ID);
      if(user == NULL || pass == NULL){
        bot.sendMessage(bot.message[i][4], F("Missing Fields - Correct syntax: /config:<User>:<Pass>"), "");
      }else if(user.equals(user_saved) && pass.equals(pass_saved)){
        chat_id = bot.message[i][4];
        bot_activated = true;
        EEPROM.write(8, bot_activated);
        EEPROM.commit();
        int index = writeStringToEEPROM(9, chat_id);
        Serial.print("Chat ID Wrote on address: ");
        Serial.println(index);
        bot.sendMessage(bot.message[i][4], F("Access Granted!"), "");
      }else{
        bot.sendMessage(bot.message[i][4], F("Bad Credentials!"), "");
      }
      
    } else if (messageRcvd.equals("/info")) {
      if(bot.message[i][4].equals(chat_id)){
        String msg = "===== System Info =====%0AMAC Address: " + MAC_ADDR + "%0A" +
                     "Contrac Status: " + (active_contract == true ? "Active" : "Expired") + "%0A" +
                     "Setted Key: " + String(setted_key[0]) + String(setted_key[1]) + String(setted_key[2]) + String(setted_key[3]) + "%0A" +
                     "Username: User_" + String(CLIENT_ID) + "%0A" +
                     "====================";
        bot.sendMessage(chat_id, msg, "");
      }else{
        bot.sendMessage(bot.message[i][4], F("Login Required!"), "");
      }
      
    }
    else if (messageRcvd.equals("/status")) {
      if(bot.message[i][4].equals(chat_id)){
        String msg = "===== System Status =====%0ALocation: " + String(WEATHER_CITY) + "%0A" +
                     "Alarm Status: " + (armed == true ? "Armed" : "Disarmed") + "%0A" + 
                     "======================";
        bot.sendMessage(chat_id, msg, "");
        
      }else{
        bot.sendMessage(bot.message[i][4], F("Login Required!"), "");  
      }

    }
    else if (messageRcvd.equals("/help")) {
      String welcome = "Welcome from Surveillance_System_Bot!";
      String welcomeCmd1 = "/config:<User>:<Pass> : to login to the bot";
      String welcomeCmd2 = "/info : to get general system info";
      String welcomeCmd3 = "/status : to get the status of your alarm";
      bot.sendMessage(bot.message[i][4], welcome, "");
      bot.sendMessage(bot.message[i][4], welcomeCmd1, "");
      bot.sendMessage(bot.message[i][4], welcomeCmd2, "");
      bot.sendMessage(bot.message[i][4], welcomeCmd3, "");
    } else {
      bot.sendMessage(bot.message[i][4], F("Unknown command! Use /help to see all the available commands"), "");
    }
  }
  bot.message[0][0] = "";   // all messages have been replied, reset new messages
}

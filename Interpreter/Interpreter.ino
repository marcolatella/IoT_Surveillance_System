
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>

#include "secrets.h"

// WiFi cfg
char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
#ifdef IP
IPAddress ip(IP);
IPAddress subnet(SUBNET);
IPAddress dns(DNS);
IPAddress gateway(GATEWAY);
#endif


// MQTT data
MQTTClient mqttClient;                                              // handles the MQTT communication protocol
WiFiClient networkClient;  // handles the network connection to the MQTT broker
#define MQTT_TOPIC_ROOT "surv_sys/hs/"
#define MQTT_TOPIC_CONFIGURATION "surv_sys/hs/config"     
#define MQTT_TOPIC_ARMED "surv_sys/hs/+/armed"            
#define MQTT_TOPIC_ALARMED "surv_sys/hs/+/alarmed"  
#define MQTT_TOPIC_CONTROL "surv_sys/hs/+/control"  
#define MQTT_TOPIC_CONTRACT "surv_sys/hs/+/contract"
#define MQTT_TOPIC_CONNECT "surv_sys/hs/+/connect" 
#define MQTT_TOPIC_LW "surv_sys/hs/+/last_will"
#define MQTT_TOPIC_RESET_RFID "surv_sys/hs/reset_rfid"

WiFiClient client;
HTTPClient http;
ESP8266WebServer server(80);

void setup() {
  WiFi.mode(WIFI_STA);
  mqttClient.begin(MQTT_BROKERIP, 1883, networkClient);   // setup communication with MQTT broker
  mqttClient.onMessage(mqttMessageReceived);
  server.on("/nodes/control", handle_system);
  server.on("/nodes/contract", handle_contract);
  server.on("/nodes/reset", handle_reset);

  server.begin();
  Serial.println(F("HTTP server started"));
  Serial.begin(115200);
  Serial.println(F("\n\nSetup completed.\n\n"));

}

void loop() {
  connectToWiFi();
  server.handleClient();
  connectToMQTTBroker();
  mqttClient.loop();
  

}

void connectToWiFi(){
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("Connecting to SSID: "));
    Serial.println(SECRET_SSID);

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


void printWifiStatus() {
  Serial.println(F("\n=== WiFi connection status ==="));

  // SSID
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // signal strength
  Serial.print(F("Signal strength (RSSI): "));
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  // current IP
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());

  // subnet mask
  Serial.print(F("Subnet mask: "));
  Serial.println(WiFi.subnetMask());

  // gateway
  Serial.print(F("Gateway IP: "));
  Serial.println(WiFi.gatewayIP());

  // DNS
  Serial.print(F("DNS IP: "));
  Serial.println(WiFi.dnsIP());

  Serial.println(F("==============================\n"));
}

void connectToMQTTBroker() {
  if (!mqttClient.connected()) {   // not connected
    Serial.print(F("\nConnecting to MQTT broker..."));
    while (!mqttClient.connect(MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.print(F("."));
      delay(1000);
    }
    Serial.println(F("\nConnected!"));

    // connected to broker, subscribe topics
    mqttClient.subscribe(MQTT_TOPIC_CONFIGURATION);
    mqttClient.subscribe(MQTT_TOPIC_ARMED);
    mqttClient.subscribe(MQTT_TOPIC_ALARMED);
    mqttClient.subscribe(MQTT_TOPIC_CONTROL);
    mqttClient.subscribe(MQTT_TOPIC_CONTRACT);
    mqttClient.subscribe(MQTT_TOPIC_CONNECT);
    mqttClient.subscribe(MQTT_TOPIC_LW);
    Serial.println(F("\nSubscribed to Configuration topic!"));
    Serial.println(F("\nSubscribed to Armed topic!"));
    Serial.println(F("\nSubscribed to Alarmed topic!"));
    Serial.println(F("\nSubscribed to Control topic!"));
    Serial.println(F("\nSubscribed to Contract topic!"));
    Serial.println(F("\nSubscribed to Connect topic!"));
    Serial.println(F("\nSubscribed to Reset RFID topic!"));
  }
}

void mqttMessageReceived(String &topic, String &payload) {
  // this function handles a message from the MQTT broker
  //Serial.println("Incoming MQTT message: " + topic + " - " + payload);

  if (topic == MQTT_TOPIC_CONFIGURATION) {
    // deserialize the JSON object
    StaticJsonDocument<128> doc;
    deserializeJson(doc, payload);
    const char *userID = doc["userID"];
    const char *mac = doc["mac"];

    String url = "http://192.168.1.11:8080/surv-sys/"+ String(userID) +"/"+ String(mac) +"/config";
    Serial.print(F("FROM: "));
    Serial.print(String(userID));
    Serial.println(F(" TO SERVER"));
    sendHTTPRequest(url);

  }else if(strstr(topic.c_str(), "alarmed") != NULL){
    StaticJsonDocument<128> doc;
    deserializeJson(doc, payload);
    const char *userID = doc["userID"];
    const char *mac = doc["mac"];
    const char *alarmed = doc["alarmed"];
    Serial.println(payload);
    String url = "http://192.168.1.11:8080/surv-sys/"+ String(userID) +"/"+ String(mac) + "/"+ String(alarmed) + "/alarm";
    Serial.print(F("FROM: "));
    Serial.print(String(userID));
    Serial.println(F(" TO SERVER"));
    sendHTTPRequest(url);
    
  } else if(strstr(topic.c_str(), "armed") != NULL){
    StaticJsonDocument<128> doc;
    deserializeJson(doc, payload);
    const char *userID = doc["userID"];
    const char *mac = doc["mac"];
    const char *armed = doc["armed"];
    Serial.println(payload);
    String url = "http://192.168.1.11:8080/surv-sys/"+ String(userID) +"/"+ String(mac) + "/"+ String(armed) + "/config";
    Serial.print(F("FROM: "));
    Serial.print(String(userID));
    Serial.println(F(" TO SERVER"));
    sendHTTPRequest(url);
    
  }else if(strstr(topic.c_str(), "connect") != NULL){
    StaticJsonDocument<128> doc;
    deserializeJson(doc, payload);
    const char *userID = doc["ID"];
    const char *mac = doc["m"];
    const char *connection = doc["c"];
    String url = "http://192.168.1.11:8080/surv-sys/"+ String(userID) +"/"+ String(mac) + "/"+ String(connection) + "/connection";
    Serial.print(F("FROM: "));
    Serial.print(String(userID));
    Serial.println(F(" TO SERVER"));
    sendHTTPRequest(url);
  }else if(strstr(topic.c_str(), "last_will") != NULL){
    StaticJsonDocument<128> doc;
    deserializeJson(doc, payload);
    const char *userID = doc["ID"];
    const char *mac = doc["m"];
    String url = "http://192.168.1.11:8080/surv-sys/"+ String(userID) +"/"+ String(mac) + "/"+ String(false) + "/connection";
    Serial.print(F("FROM: "));
    Serial.print(String(userID));
    Serial.println(F(" TO SERVER"));
    sendHTTPRequest(url);
    
  }else{
    Serial.println(F("MQTT Topic not recognized, message skipped"));
  }
}

void sendHTTPRequest(String url){
  http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) { //Check the returning code
      String payload = http.getString();   //Get the request response payload
      Serial.println(payload);             //Print the response payload
    }else{
      Serial.print("Error:");
      Serial.println(httpCode);
    }

    http.end();
}

void handle_system() {
  const char* tmp = server.arg(0).c_str();
  String payload = String(tmp);
  sendFromServerToBrokerSystem(payload);
  server.send(200, F("text/html"), "Received Data:" + payload);
}

void handle_contract() {
  const char* tmp = server.arg(0).c_str();
  String payload = String(tmp);
  sendFromServerToBrokerContract(payload);
  server.send(200, F("text/html"), "Received Data:" + payload);
}

void handle_reset(){
  const char* tmp = server.arg(0).c_str();
  String payload = String(tmp);
  sendFromServerToBrokerResetRFID(payload);
  server.send(200, F("text/html"), "Received Data:" + payload);
}


void sendFromServerToBrokerContract(String payload){
  String userID = splitString(payload,' ',0);
  String mac = splitString(payload,' ',1);
  String contract = splitString(payload,' ',2);
  const int capacity = JSON_OBJECT_SIZE(10);
  StaticJsonDocument<capacity> doc;
  doc["userID"] = userID;
  doc["mac"] = mac;
  doc["contract"] = contract;
  char buffer[256];
  size_t n = serializeJson(doc, buffer);
  Serial.print(F("FROM SERVER TO BROKER: "));
  Serial.println(buffer);
  String topic_to_publish = MQTT_TOPIC_ROOT + mac + "/contract";
  mqttClient.publish(topic_to_publish.c_str(), buffer, n, true);
}

void sendFromServerToBrokerSystem(String payload){
  String userID = splitString(payload,' ',0);
  String mac = splitString(payload,' ',1);
  String armed = splitString(payload,' ',2);
  const int capacity = JSON_OBJECT_SIZE(10);
  StaticJsonDocument<capacity> doc;
  doc["userID"] = userID;
  doc["mac"] = mac;
  doc["armed"] = armed;
  char buffer[256];
  size_t n = serializeJson(doc, buffer);
  Serial.print(F("FROM SERVER TO BROKER: "));
  Serial.println(buffer);
  String topic_to_publish = MQTT_TOPIC_ROOT + mac + "/control";
  mqttClient.publish(topic_to_publish.c_str(), buffer, n);
}

void sendFromServerToBrokerResetRFID(String payload){
  String mac = splitString(payload,' ',0);
  const int capacity = JSON_OBJECT_SIZE(10);
  StaticJsonDocument<capacity> doc;
  doc["mac"] = mac;
  doc["reset"] = true;
  char buffer[256];
  size_t n = serializeJson(doc, buffer);
  Serial.print(F("FROM SERVER TO BROKER: "));
  Serial.println(buffer);
  String topic_to_publish = MQTT_TOPIC_ROOT + mac + "/reset_rfid";
  mqttClient.publish(topic_to_publish.c_str(), buffer, n);
}

String splitString(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

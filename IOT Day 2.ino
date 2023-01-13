#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <String.h>
WebSocketsClient webSocket;
const char *ssid     = "Mr_Jotin";
const char *password = "24240000";
unsigned long messageInterval = 5000;
bool connected = false;
#define DEBUG_SERIAL Serial
// gpio pinout define
//relay
#define DHTPIN D2
#define FAN D1
#define MYLED D0
// const int Fan = D0;
// const int Led = D2;
#define DHTTYPE    DHT11 
//dht sensor
DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;


//Variable i wnat to save in eeprom
//1. Connection Key Example 192.168.43.4#8005#/ws/ClientRoom/suman-BedRoom-device1
//2. Secret Key
//3. SSID for network
//4. PSK for network
//5. ConnectionID

//JWT ENCODER 
DynamicJsonDocument doc(1024);

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            DEBUG_SERIAL.printf("[WSc] Disconnected!\n");
            connected = false;
            break;
        case WStype_CONNECTED: {
           // DEBUG_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
            connected = true;
 
            // send message to server when Connected
            DEBUG_SERIAL.println("[WSc] SENT: Connected");
            webSocket.sendTXT("Connected");
        }
            break;
        case WStype_TEXT:
//            DEBUG_SERIAL.printf("[WSc] RESPONSE: %s\n", payload);
            handleReceivedMessage(payload);
            break;
        case WStype_BIN:
            DEBUG_SERIAL.printf("[WSc] get binary length: %u\n", length);
            hexdump(payload, length);
            break;
                case WStype_PING:
                        // pong will be send automatically
                        DEBUG_SERIAL.printf("[WSc] get ping\n");
                        break;
                case WStype_PONG:
                        // answer to a ping we send
                        DEBUG_SERIAL.printf("[WSc] get pong\n");
                        break;
    }
 
}
// incomming Json Data to usable data
void handleReceivedMessage(uint8_t * message)
{
  String strMessage = String((char*)message);
  String result = strMessage.substring(1, strMessage.length() - 1);
  char input2[200];
  result.toCharArray(input2,200);
  StaticJsonDocument<200> doc;                     //Memory pool
  DeserializationError error = deserializeJson(doc, strMessage); //Parse message
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  Serial.println(int(doc["id"]));
  const int id = doc["id"];
  const char* sta = doc["status"];
  const char* data = doc["data"];
////  do some task according to the action

   String statusCommand = String(sta);
   String dataS = String(data);
   if(statusCommand =="rec")//send
   {
      Serial.println("Data is sending from the Devices");
   }
   else if(statusCommand == "sen")//recieve
   {
      sendCommand2Server(dataS);
   }
}
void setup() {

  //configuring gpio pinout
    pinMode(MYLED, OUTPUT);
    pinMode(FAN, OUTPUT);
    dht.begin();
    DEBUG_SERIAL.begin(115200);
 
//  DEBUG_SERIAL.setDebugOutput(true);
 
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println();
    
    for(uint8_t t = 4; t > 0; t--) {
        DEBUG_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        DEBUG_SERIAL.flush();
        delay(1000);
    }
 
    WiFi.begin(ssid, password);
 
    while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 500 );
      Serial.print ( "." );
    }
    DEBUG_SERIAL.print("Local IP: "); DEBUG_SERIAL.println(WiFi.localIP());
    // server address, port and URL
     webSocket.begin("newbiequest.in", 80, "/ws/ClientAdmin/test/DemoRoom-DemoSubroom-device1/"); //Channel name
 
    // event handler
    webSocket.onEvent(webSocketEvent);
}
 
unsigned long lastUpdate = millis();
 
 
void loop() {
    
    webSocket.loop();
    if (connected && lastUpdate+messageInterval<millis()){
        delay(delayMS);
        sensors_event_t event;
        dht.temperature().getEvent(&event);
    
          doc["id"] = 2;
          doc["status"]   = "rec";
          doc["data"] = int(event.temperature);
          char outputData[100];
          serializeJson(doc, outputData);
          webSocket.sendTXT(outputData);
          lastUpdate = millis();//delay(1000)
        
        
    }
}

// send data to server
void sendCommand2Server(String data)
{
  int delimiter,delimiter1,delimiter2;
  delimiter = data.indexOf('#');
  delimiter1 = data.indexOf('#',delimiter+1);
  delimiter2 = data.indexOf('#',delimiter1+1);
  String pinName = data.substring(delimiter +1,delimiter1);
  String pinStatus = data.substring(delimiter1+1,delimiter2);
  Serial.println(pinName);
  Serial.println(pinStatus);
  if(pinStatus == "on")
  {
    if(pinName == "D1"){
      digitalWrite(FAN, 1);}
    else if(pinName == "D0"){
      digitalWrite(MYLED, 1);}
  }
  else if(pinStatus == "off")
  {
    if(pinName == "D1"){
      digitalWrite(FAN, 0);}
    else if(pinName == "D0"){
      digitalWrite(MYLED, 0);}
  }
}
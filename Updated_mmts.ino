#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#define LED D0            // Led in NodeMCU at pin GPIO16 (D0).
#define WIFI_LED D1
// WiFi
const char *ssid = "Sri Vinayaka SMP PG 2"; // Enter your WiFi name
const char *password = "88861564492";  // Enter WiFi password 74140749
// MQTT Broker
const char *mqtt_broker = "broker.hivemq.com";
const char *topic = "starttopic";
const char *diag_topic ="diagtopic";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);
   pinMode(LED, OUTPUT);
    pinMode(WIFI_LED,OUTPUT);
    digitalWrite(WIFI_LED,LOW);
   digitalWrite(LED,LOW);
   pinMode(2,OUTPUT);  
    
    
  // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  digitalWrite(WIFI_LED,HIGH);
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  client.setCallback(diagcallback);
  while (!client.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
    //  Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str())) {
           Serial.println("Public  mqtt broker connected");
          digitalWrite(WIFI_LED,HIGH);
      } else {
        //  Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }
  // publish and subscribe
  client.subscribe("starttopic");
  client.subscribe("diagtopic");
 
}

void callback(char *topic, byte *payload, unsigned int length) {
  int i;
 Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");
  for (i = 0; i < length; i++)
  {
         Serial.print((char)payload[i]);
  }
 Serial.println();
  
}
void diagcallback(char *diag_topic, byte *payload, unsigned int length) {
  int i;
 Serial.print("Received [");
  Serial.print(diag_topic);
  Serial.print("]: ");
  for (i = 0; i < length; i++)
  {
         Serial.print((char)payload[i]);
  }
 Serial.println();
  
}


void loop() {
  client.loop();
   if(Serial.available())
  {
    char rec = Serial.read();
    switch(rec)
    {
      case 0:
      Training_mode_pub(0);
      break;
      case 1:
      Training_mode_pub(1);
      break;
      case 2:
      Diagnostic_mode_pub("DMT");
      break;
      case 3:
      Diagnostic_mode_pub("DLT");
      break;
      case 4:
      Diagnostic_mode_pub("DLS");
      break;
      default:
      break;

    }

  }
}
// void wait(char data[])
// {
//   int inc=0;
//   char arr[3];
//   do{
//       arr[inc++]=Serial.available();
//   }while(arr != data);
// }

void Training_mode_pub(char rec)
{
  StaticJsonDocument<200> doc;
  doc["status"] =rec;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
  client.publish("resulttopic", jsonBuffer);
  digitalWrite(BUILTIN_LED,HIGH);
  delay(500);
  digitalWrite(BUILTIN_LED,LOW);
  delay(500);
}
void Diagnostic_mode_pub(String rec)
{
  StaticJsonDocument<200> doc;
  doc["diagresult"] =rec;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
  client.publish("diagresult", jsonBuffer);
}




  

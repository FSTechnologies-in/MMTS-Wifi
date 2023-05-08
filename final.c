#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <string>
#include <cstring>
#define LED D0            // Led in NodeMCU at pin GPIO16 (D0).
#define WIFI_LED D1
#define SIZE 512
#define EEPRM_ERACE D4
//Variables
int i = 0;
int statusCode;
const char* ssid = "default";
const char* passphrase = "default";
const char* local_ip;
String st;
String content;
const char *topic = "starttopic";
const int mqtt_port = 1883;
int buttonState=0;
int solar=A0;
int ir_status=0;
int buff[20]; 
int diff=0;
int a=0,b[20];
int count=0;
int c;
int solar_flag=0;
WiFiClient espClient;
PubSubClient client(espClient);

//Function Decalration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);

//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);

void setup()
{

  Serial.begin(9600); //Initialising if(DEBUG)Serial Monitor
  Serial.println();
   pinMode(LED, OUTPUT);
    pinMode(solar,INPUT);
    pinMode(WIFI_LED,OUTPUT);
    digitalWrite(WIFI_LED,LOW);
   digitalWrite(LED,LOW);
   pinMode(2,OUTPUT); 
   pinMode(EEPRM_ERACE, INPUT_PULLDOWN_16);
  Serial.println("Disconnecting current wifi connection");
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println();
  Serial.println();
  Serial.println("Startup");
  client.setCallback(callback);
  //---------------------------------------- Read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");

  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 64; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  Serial.println("Reading EEPROM ip address");
 
  char eipadd[13];
  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");
  int k=0;
  for (int i = 64; i < 111; ++i)
  {
    eipadd[k++] = char(EEPROM.read(i));
  }
  k=0;
   Serial.print("IP ADDRESS: ");
  Serial.println(eipadd);
  client.setServer(eipadd, mqtt_port);
  
  while (!client.connected()) {

      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
    //  Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str())) {
           Serial.println("Public  mqtt broker connected");
          

      } else {
        //  Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
          Serial.println("clearing eeprom");
        for (int i = 0; i < 120; ++i) {
          EEPROM.write(i, 0);
        }
        EEPROM.commit();
        ESP.reset();    
        digitalWrite(WIFI_LED,HIGH);

         
      }
  }
  // publish and subscribe
  client.subscribe("starttopic");
    return;
  }
  else
  {
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();// Setup HotSpot
  }

  Serial.println();
  Serial.println("Waiting.");

  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }

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
  solar_flag = 1;
 Serial.println();
  
}
void loop() {
  if ((WiFi.status() == WL_CONNECTED))
  {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);

  }
  else
  {
  }
  if(solar_flag==1)
  {
      
  solar_fun();
  }
  
  if(digitalRead(EEPRM_ERACE) == HIGH)
  {
       Serial.println("clearing eeprom");
        for (int i = 0; i < 120; ++i) {
          EEPROM.write(i, 0);
        }
        EEPROM.commit();
        ESP.reset();          
  }
  client.loop();
   if(Serial.available())
  {
    
    int increment=0;
    int count=0;
    char data_buffer;
    String receive="";
    char buff[140];
    String id_val="";
    memset(buff,0,sizeof(buff));
    if(Serial.available())
    {
       for(int t=0;t<30;t++)
       {
           data_buffer = Serial.read();
           receive += data_buffer;
          
       }
    }      
     receive.toCharArray(buff, 140);
      Serial.print("Data:");
      Serial.println(buff);          
      Extract(buff);
      
      
        
    solar_flag=0;
   
  }

}
void publishMessage(char ID[],char name[],char result[])
{
  StaticJsonDocument<200> doc;
  doc["id"]=ID;
  doc["name"]=name;
  doc["status"] =result;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
  client.publish("resulttopic", jsonBuffer);
  digitalWrite(BUILTIN_LED,HIGH);
  delay(500);
  digitalWrite(BUILTIN_LED,LOW);
  delay(500);
}
void Extract(char data_buffer[])
{
  int id_increment=0;
    int name_increment=0;
    int status_increment=0;
   int count_1=0;
   int count_2=0;
   int count_3=0;
   char name[30];
    char ID[30];
    char result[3];
    memset(ID,0,sizeof(ID));
      memset(name,0,sizeof(name));
      memset(result,0,sizeof(result));
     if(data_buffer[count_1]=='#')
     {
         while(data_buffer[++count_1]!=','){
      
          ID[id_increment++]=data_buffer[count_1];
          
         }
     }
    //  Serial.println();
    //  Serial.print(ID);
     if(data_buffer[count_1+1]=='@')
     {
          while(data_buffer[++count_1]!=','){
          name[name_increment++]=data_buffer[count_1+1];
         
         }
     }
    //  Serial.println();
    //  Serial.print(name);
     if(data_buffer[count_1+1]=='$')
     {
          while(data_buffer[++count_1]!=','){
          result[status_increment++]=data_buffer[count_1+1];
         }
     }
    memset(data_buffer,0,sizeof(data_buffer));
    char id_ptr[id_increment+1];
    char name_ptr[name_increment];
    char status_ptr[status_increment+1];
    // Serial.print("Inside loop ID");
    for(int g=0;g<=id_increment-1;g++)
    {
      id_ptr[g]=ID[g];
    //  Serial.print(ID[g]); 
    }
    Serial.println();
    // Serial.print("Inside loop Name");
    for(int g=0;g<=name_increment-2;g++)
    {
      name_ptr[g]=name[g];
      //  Serial.print(name[g]);      
    }
    // Serial.print("Inside loop Result");
    for(int g=0;g<status_increment-1;g++)
    {
      status_ptr[g]=result[g];
      //  Serial.print(result[g]);      
    }
    
     Serial.println();
    
     Serial.println(id_ptr);
     Serial.println(name_ptr);
     Serial.println(status_ptr);
    
     publishMessage(id_ptr,name_ptr,status_ptr);
     memset(id_ptr,0,sizeof(id_ptr));
      memset(name_ptr,0,sizeof(name_ptr));
      memset(status_ptr,0,sizeof(status_ptr));
      memset(ID,0,sizeof(ID));
      memset(name,0,sizeof(name));
      memset(result,0,sizeof(result));
     
}
void solar_fun()
{
  // digitalWrite(LED,LOW);
     ir_status=analogRead(solar);
      buff[count]=ir_status;
      // Serial.print("Value: ");
         Serial.println(buff[count]);
         delay(500);
     if(count>0)
     {
        c= buff[count]-buff[count-1];
        // Serial.print("Value: ");
        // Serial.println(c);
         delay(50);
        if(c>=12&&c<23)
        {
             digitalWrite(LED,HIGH);
             Serial.println("Shot success");
             delay(1000);
                    }
        else
        {
          digitalWrite(LED,LOW);
        }
        
     }
     if(count>19)
        count=0;
    count++;
}

//----------------------------------------------- Fuctions used for WiFi credentials saving and connecting to it which you do not need to change
bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}


void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("MMTS", "12345678");
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  launchWeb();
  Serial.println("over");
}

void createWebServer()
{
  {
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Welcome to Wifi Credentials Update page";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><label>PASSWORD: </label><input name='pass' length=64><label>IP ADDRESS: </label><input name='local_ip' length=95><input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      String ipadd = server.arg("local_ip");
      if (qsid.length() > 0 && qpass.length() > 0 && ipadd.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 120; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");
        Serial.println(local_ip);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        Serial.println("writing eeprom ip address:");
        for (int i = 0; i < ipadd.length(); ++i)
        {
          EEPROM.write(64 + i, ipadd[i]);
          Serial.print("Wrote: ");
          Serial.println(ipadd[i]);
        }
        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.reset();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);

    });
  }
}

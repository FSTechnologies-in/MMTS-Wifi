#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>  //Ticker Library

#define LED D0            // Led in NodeMCU at pin GPIO16 (D0).
#define WIFI_LED D1
#define SW1 D2
#define SW2 D3
#define Motor_Input D4
#define Motor_Direction D5
#define Buzzer D6
#define Laser D7

Ticker secondTicker; // Ticker object to handle the timer
volatile unsigned long secondsCounter = 0; // Counter to store the seconds
bool timerRunning = false;


// Function Prototype
int data_from_iot(void);
void motor_drive_clockwise(void);
void motor_drive_anticlockwise(void);
int iot_data_validation(char data[]);
void switch1_release(void);
void laser_detect(void);
void timer_start(void);
void Buzzer_function(void);
void motor_drive_stop(void);
void motor_diagnostic(void);
int led_diagnostic(void);
void laser_diagnostic(void);
// WiFi
const char *ssid = "Redmi Note 7"; // Enter your WiFi name
const char *password = "12345678";  // Enter WiFi password 74140749
// MQTT Broker
const char *mqtt_broker = "broker.hivemq.com";
const char *topic = "starttopic";
const char *diag_topic ="diagtopic";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);
char mqtt_payload[150];
/* Global Variable */

char User_Name[30];
char Mode_of_Rotation[30];
char Time_Duration[30];
unsigned long Time_from_Iot;
char data;
char buff1[150];
char buff2[90];
char clockwise[20];
char time_buff[20];
uint8_t count_sec = 0;
uint8_t flag=0;
uint8_t stop=0;
uint8_t tim_val = 0;
uint8_t time_flag=0;
uint8_t sw1_flag=0;
uint8_t sw2_flag=0;
uint8_t time_end_flag=0;
uint8_t iot_flag=0;
uint8_t event_flag=0;
uint8_t time_start=0;
uint8_t mode=0;
void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);
   pinMode(LED, OUTPUT);
    pinMode(WIFI_LED,OUTPUT);
    pinMode(SW1,INPUT_PULLDOWN_16);
    pinMode(SW2,INPUT_PULLDOWN_16);
    pinMode(Motor_Direction,OUTPUT);
    pinMode(Motor_Input,OUTPUT);
    pinMode(Buzzer,OUTPUT);
    pinMode(Laser,OUTPUT);
    pinMode(LED_BUILTIN,OUTPUT);
    digitalWrite(LED_BUILTIN,LOW);
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
        // mqtt_payload[i];
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
        // mqtt_payload[i];
         Serial.print((char)payload[i]);
  }
 Serial.println();
  
}


void loop() {

   if (iot_flag == 0)
	{
		//  if IoT flag is clear then entering to Data_from_iot
		if (data_from_iot())    // Data_from_IoT function return 1 then set the I0T flag
		    iot_flag = 1;
	}

  if (iot_flag)
	{
		//if IoT flag is set enter into the  function
		 /*
		 * initially switch-1 flag is zero,
		 * then motor start running from default position to 90 degree position (anti-clock wise direction).
		 */
		if ( event_flag == 0)
		{
			event_flag++;
			Serial.println("anticlockwise motor start");
			motor_drive_clockwise();
		}

	         /*
		* once motor touch switch-2 then motor stop, timer starts for 10 seconds,
		* and motor also stay in 90 degree position
		*/
		if (digitalRead(SW2) == 1 && event_flag == 1)
		{
    event_flag++;
		 Serial.println("motor stop\r\n");
			motor_drive_stop();
			delay(1000);
			timer_start();
		}
	     /*if laser is not short until after 10 seconds,
	     * then motor start running from 90 degree position to default position(clockwise direction)
	     */
		 /*
		 * Once motor touch the switch-1 again(default position) stop the motor
		 */
	   if (digitalRead(SW1) == 1 && event_flag == 2)
            {
		time_end_flag=0;
		Serial.println("default position");
		motor_drive_stop();
		iot_flag = 0;
		event_flag=0;
	   }
	 /*
	 *  once the timer flag is set call the laser_detect function
	 *  if timer flag is clear then stop transmitting the RF signal
	 */
	if (time_flag == 1)
	{
		laser_detect();
	}
	/*
	 * Timer flag is reset in timer interrupt to disable the Rf tx pin
	 */

	if(time_end_flag)
	{
	
		motor_drive_anticlockwise();
	}
  /* IoT timer and Software timer is Equal, There Is no shoot Acquired */
  if (timerRunning && Time_from_Iot == secondsCounter) {
    stopTimer();
  }  

}

  client.loop();
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
int data_from_iot(void)
{
	/*
	 * checking for IoT data,checking for name,mode,dutation of time;
	 */

	uint8_t count=0;
	uint8_t quote_double=0;
	uint8_t name_increment=0;
	uint8_t mode_increment=0;
  uint8_t time_increment=0;

    strcpy(buff1,mqtt_payload);

		while (1)
		{
		      if (buff1[count] == '"')
		      {
			    ++quote_double;
		      }

		     if (quote_double == 3)
		     {
			   if (buff1[count + 1] != '"')
			   {
				User_Name[name_increment] = buff1[count + 1];
				name_increment++;
			   }
			   else
			   {
				   if (buff1[count + 2] == '}')
					  {
					   quote_double=0;
					   sprintf(buff2, "Name:%s\r\n", User_Name);
						  mode=2;
						  break;
					  }
			   }
		    }

		    if (quote_double == 7)
		    {
			   if (buff1[count + 1] != '"')
			   {
				Mode_of_Rotation[mode_increment] = buff1[count + 1];
				mode_increment++;
			   }
		   }

		   if (quote_double == 11)
		   {
			  if (buff1[count + 1] != '"')
			  {
				Time_Duration[time_increment] = buff1[count + 1];
				time_increment++;
			  }
			  else
			  {

			  }
		   }
		   if (quote_double == 14)
		   		   {
			         quote_double=0;
				   	   mode=1;
			  					  break;
		   		   }
//		  if (quote_double == 12)
//		  {
//			  mode=2;
//			break;
//		  }
		  ++count;
          }

	Time_from_Iot = atoi(Time_Duration);
	sprintf(buff2,"Mode:%d\r\n",Time_from_Iot);
  Serial.print("Outside Task Time:");
  Serial.println(buff2);
  memset(buff2,0,sizeof(buff2));
	if (mode == 1) {
				Time_from_Iot = atoi(Time_Duration);
				sprintf(buff2, "Time:%d\r\n", Time_from_Iot);
				if (strcmp(Mode_of_Rotation, "Slice") == 0 && Time_from_Iot) {
					memset(Mode_of_Rotation, 0, sizeof(Mode_of_Rotation));
					sprintf(buff2, "Duration:%d\r\n", Time_from_Iot);
					iot_flag = 1;
					return 1;
				} else {
					return 0;
				}
			}
	if(mode==2)
	{

		if(strcmp(User_Name,"MT") ==0)
			{
				//call motor function
				motor_diagnostic();
				 Diagnostic_mode_pub("DMT");
				return 0;
			}
		 if(strcmp(User_Name,"LT") ==0)
			{
				//call led function
				led_diagnostic();
				Diagnostic_mode_pub("DLT");

			}
		 if(strcmp(User_Name,"LS") ==0)
			{
				//call laser function
				laser_diagnostic();
				return 0;
			}
			else
			{
				return 0;
			}
	}
	return 0;
     
}
void motor_drive_clockwise(void)
{
	/*
	 * start motor clockwise to reach Default position
	 * Motor Input 1 - SET
	 * Motor Input 2 - RESET
	 */

	Serial.println("clock direction");
	analogWrite(Motor_Input, 40);
	digitalWrite(Motor_Input, HIGH); //Motor ON
	digitalWrite(Motor_Direction, LOW); //Direction of Pin(CLOCK WISE)
}

void motor_drive_anticlockwise(void)
{
	/*
	 * start motor anti - clockwise to reach 90 degree position
	 * Motor Input 1- RESET
	 * Motor Input 2- SET
	 */

	Serial.println("Counter clock wise");
	analogWrite(Motor_Input, 40);
	digitalWrite(Motor_Input, HIGH); //Motor ON
	digitalWrite(Motor_Direction, HIGH); //Direction of Pin(CLOCK WISE)
}

void motor_drive_stop(void)
{
	 /*
	 * turn off the motor
	 * Motor Input 1- Reset
	 * Motor Input 2- Reset
	 */
	Serial.println("Motor stop");
	digitalWrite(Motor_Input, HIGH); //Motor ON
}

void timer_start(void)
{
	  /*
	  *  start timer for 10 seconds and buzzer on
	  */

  Serial.println("time Start");
	Buzzer_function();
  if (!timerRunning && Time_from_Iot > 0) { // start timer
    startTimer();
  }
	time_flag = 1; // Set Timer flag to indicate the timer on call laser detect function in superloop
}

void Buzzer_function(void)
{
	 /*
	 *  buzzer to indicate timer as started
	 */
	digitalWrite(Buzzer, HIGH);
	delay(500);
	digitalWrite(Buzzer, LOW);
}

void laser_detect(void)
{
	 /*
	 * as timer started meanwhile start detecting the laser gun is short r not,
	 * if laser gun is short then move motor to default position (clock-wise direction),
	 * stop the timer.
	 *
	 */
	if (digitalRead(Laser) == 1)
	{
    stopTimer();// Once laser detected stop the timer
		time_flag = 0; // Reset timer flag for another time will execute this function
		Serial.println("laser Function");
		Buzzer_function();
		Training_mode_pub(1);
		memset(Time_Duration,0,sizeof(Time_Duration));
		delay(5000);
		motor_drive_anticlockwise();
         sw1_flag=1;// Increment sw flag to turn on switch sw2
	     	count_sec=0;

	}

}

void motor_diagnostic(void)
{
	analogWrite(Motor_Input,40);
	digitalWrite(Motor_Direction,LOW);
	digitalWrite(Motor_Input,HIGH); //Motor ON
  while(1)
  {

	if (digitalRead(SW2) == 0)
	{
		digitalWrite(Motor_Input,LOW); //Motor ON //Motor OFF
		delay(1000);
	analogWrite(Motor_Input,25);
	digitalWrite(Motor_Direction,HIGH);//Direction of Pin(COUNTER CLOCK WISE)
	digitalWrite(Motor_Input,HIGH); //Motor ON
	delay(1000);

	}
	if(digitalRead(SW1) == 0)
	{
			digitalWrite(Motor_Input,LOW); //Motor OFF
		  mode=0;
			iot_flag = 0;
		break;
	}

  }

}
int led_diagnostic(void)
{

	for(int led=0;led<5;led++)
	{
		digitalWrite(LED_BUILTIN,HIGH);
		delay(300);
    digitalWrite(LED_BUILTIN,LOW);
	}
	memset(User_Name,0,sizeof(User_Name));
	mode=0;
	iot_flag = 0;
	return 0;
}
void laser_diagnostic(void)
{
	volatile long int delay_inc=0;
	int count_d=0;
	int shoot_flag=0;

	while(1)
	{
		delay_inc++;
		if(delay_inc==100000)
		{
			delay_inc=0;
			count_d++;

			if(count_d>10)
				break;
		}

		if (digitalRead(Laser) == 1)
		{
			Buzzer_function();
			shoot_flag=1;
			Diagnostic_mode_pub("DLS");
			break;
		}


	}
   if(shoot_flag == 1)
   {
     Serial.println("Shoot success");
   }

	else
			{
			Buzzer_function();
			delay(300);
			Buzzer_function();
			delay(300);
			Buzzer_function();
			delay(300);
			}

	mode=0;
    iot_flag = 0;
}

void secondCallback() {
  secondsCounter++; // Increment the seconds counter
  Serial.println("Seconds: " + String(secondsCounter));
}

// Function to start the timer
void startTimer() {
  secondTicker.attach(1, secondCallback); // Attach the ticker to start the timer
  timerRunning = true;
  Serial.println("Timer started.");
}

// Function to stop the timer
void stopTimer() {
  Training_mode_pub(0);
  secondTicker.detach(); // Detach the ticker to stop the timer
  timerRunning = false;
  Serial.println("Timer stopped.");
}



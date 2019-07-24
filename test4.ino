#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>

// Update these with values suitable for your network.

const char* ssid = "Visitor";
const char* password = "security";
const char* mqtt_server = "163.50.57.25";

WiFiClient espClient;
PubSubClient client(espClient);
LiquidCrystal_I2C lcd(0x27, 20, 4);
long lastMsg = 0;
long lastMsg2 = 0;
long lastMsg3 = 0;
char msg[50];
//int value = 0;
int wait = 0;

//sensor
// dht22
#define DHTPIN 12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float temperature;
float humidity;

//light
int lux;
float ADC_value = 0.0048828125;
float LDR_value;
int Light_Pin = A0;
//sw
int sw1 = 14;
int sw2 = 13;
int sw3 = 9;
int stateButton = 2;
//LED
int r = 0;
int y = 2;
int g = 10;

// Config time
int timezone = 7;
char ntp_server1[20] = "163.50.57.25";
int dst = 0;

String NowString() {
  time_t now1 = time(nullptr);
  struct tm* newtime = localtime(&now1);

  String tmpNow = "";
  tmpNow += "  ";
  if((newtime->tm_hour)<10){
    tmpNow += "0"+String(newtime->tm_hour);
  }else{
    tmpNow += String(newtime->tm_hour);
  }
  tmpNow += ":";
  if((newtime->tm_min)<10){
    tmpNow += "0"+String(newtime->tm_min);
  }else{
    tmpNow += String(newtime->tm_min);
  }
  tmpNow += ":";
  if((newtime->tm_sec)<10){
    tmpNow += "0"+String(newtime->tm_sec);
  }else{
    tmpNow += String(newtime->tm_sec);
  }
  return tmpNow;
}

String NowString2() {
  time_t now1 = time(nullptr);
  struct tm* newtime = localtime(&now1);

  String tmpNow = " ";
  if((newtime->tm_mday)<10){
    tmpNow += "0"+String(newtime->tm_mday);
  }else{
    tmpNow += String(newtime->tm_mday);
  }
  tmpNow += "-";
  if((newtime->tm_mon + 1)<10){
    tmpNow += "0"+String(newtime->tm_mon + 1);
  }else{
    tmpNow += String(newtime->tm_mon + 1);
  }
  tmpNow += "-";
  tmpNow += String(newtime->tm_year + 1900);
  return tmpNow;
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    //Serial.println("wifi status = " + (String)WiFi.status());
    delay(500);
    Serial.print(".");
    wait++;
    if(wait == 10){
      ESP.restart();
    }
  }
  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      ESP.restart();
      delay(5000);
    }
  }
}

void setup() {
  dht.begin();
  pinMode(sw1,INPUT_PULLUP);
  pinMode(sw2,INPUT_PULLUP);
  pinMode(sw3,INPUT_PULLUP);
  stateButton = 2;

  //LED 
  pinMode(r, OUTPUT);
  pinMode(y, OUTPUT);
  pinMode(g, OUTPUT);
  digitalWrite(r, 1);
  digitalWrite(g, 1);
  digitalWrite(y, 1);
  
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(Light_Pin,INPUT);
  lcd.begin();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Training Room");

  configTime(timezone * 3600, dst, ntp_server1);
  
}

void loop() {
  int button1 = digitalRead(sw1);
  int button2 = digitalRead(sw2);
  int button3 = digitalRead(sw3);
  if(button1 == 0){
    digitalWrite(r, 0);
    delay(100);
    digitalWrite(r, 1); //1 คือ ไฟปิด
     lcd.setCursor(0, 1);
     lcd.print("                    ");
     lcd.setCursor(0, 2);
     lcd.print("                    ");
     lcd.setCursor(0, 3);
     lcd.print("                    ");
    stateButton = 1;
    
  }
  if(button2 == 0){
    digitalWrite(r, 0);
    delay(100);
    digitalWrite(r, 1); //1 คือ ไฟปิด
    stateButton = 2;
  }
  if(button3 == 0){
    digitalWrite(r, 0);
    delay(100);
    digitalWrite(r, 1); //1 คือ ไฟปิด
    stateButton = 3;
  }
  //delay(100);
  
  if(stateButton == 1 || stateButton == 2){
    digitalWrite(g, 0);  // 0 คือไฟเปิด
  }

  
  //Serial.println("wifi status = " + (String)WiFi.status());
  if(WiFi.status() == 0 || WiFi.status() == 1 || WiFi.status() == 4 || WiFi.status() == 5 || WiFi.status() == 6 ){
    ESP.restart();
  }
  
  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();
  
  long now1 = millis();
  long now2 = millis();
  long now3 = millis();
  
    // DHT22
    humidity  = dht.readHumidity();
    temperature = dht.readTemperature(); 
    //Light
    LDR_value=analogRead(Light_Pin);
    lux=int((250.000000/(ADC_value*LDR_value))-50.000000);
    
      
    if(stateButton == 1){
      //if (now3 - lastMsg3 > 1000) {
        //lastMsg3 = now3;
        lcd.setCursor(4, 1);
        lcd.print(NowString());  
        lcd.setCursor(4, 2);
        lcd.print(NowString2());
     // }
    }
    //LCD----------------
    if (now2 - lastMsg2 > 500) {
      lastMsg2 = now2;
      if(stateButton == 2){
        lcd.setCursor(0, 1);
        lcd.print("Temp(*C): "+String(temperature));
        lcd.setCursor(0, 2);
        lcd.print("Humidity(%RH): "+String(humidity));
        lcd.setCursor(0, 3);
        if(lux<10){
          lcd.print("Light(LUX): "+String(lux)+"       ");
        }
        if(lux>=10 && lux<100){
          lcd.print("Light(LUX): "+String(lux)+"      ");
        }
        if(lux>=100 && lux<1000){
          lcd.print("Light(LUX): "+String(lux)+"     ");
        }
        if(lux>=1000 && lux<10000){
          lcd.print("Light(LUX): "+String(lux)+"    ");
        }
        if(lux>=10000 && lux<=100000){
          lcd.print("Light(LUX): "+String(lux)+"   ");
        }
        if(lux>100000){
          lcd.print("Light(LUX): NAN     ");
        }
      }
    }
    
    if(stateButton == 3){
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("   ...Restart...    ");
      delay(1500);
      ESP.restart();
    }
    
    Serial.println("Temp = "+ String(temperature));
    Serial.println("Humi = " + String(humidity));
    Serial.println("Lux = " + String(lux));
    
    String light_status;
    if(lux<40){
      light_status = "Light is off";
    }
    if(lux>150){
      light_status = "Light is on"; 
    }
    //MQTT
    if (now1 - lastMsg > 1000) {
      lastMsg = now1;
      snprintf (msg, 50, "%f",temperature);
      client.publish("Get_Temp", msg);
      snprintf (msg, 50, "%f",humidity);
      client.publish("Get_Humi", msg);
      snprintf (msg, 50, "%ld",lux);
      client.publish("Get_Light", msg);
      snprintf (msg, 50, "%s",light_status.c_str());
      client.publish("light_status", msg);
  }
}

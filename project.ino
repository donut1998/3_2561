#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 
#include "DHT.h"
#include "time.h"

// Update these with values suitable for your network.
const char* ssid = "Visitor";
const char* password = "security";
const char* mqtt_server = "163.50.57.110";

WiFiClient espClient;
PubSubClient client(espClient);

//NTPserver
const char* ntpServer = "";
const long  gmtOffset_sec = 7 *60 *60;
const int   daylightOffset_sec = 0; 

LiquidCrystal_I2C lcd(0x27, 20, 4);
long lastMsg[3] = {0,0,0};
char msg[50];
//int value = 0;
int wait = 0;

//sensor
// dht22
#define DHTPIN 12
#define DHTTYPE DHT22  
DHT dht(DHTPIN, DHTTYPE);

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

void printLocalTime(){
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y");
    Serial.println(&timeinfo, "%H:%M:%S");
    lcd.setCursor(0, 1);
    lcd.print(&timeinfo, "%A, %B %d %Y");
    lcd.setCursor(0, 2);
    lcd.print(&timeinfo, "%H:%M:%S");
    lcd.setCursor(0, 3);
    lcd.print("                    ");
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
  //button 
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
  
//  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(Light_Pin,INPUT);
  //LCD
  lcd.begin();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Training Room");
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  for(int i=0; i< (sizeof(lastMsg) / sizeof(lastMsg[0])); i++){
    lastMsg[i] = 0;
  }
}

void loop() {
  
  int button1 = digitalRead(sw1);
  int button2 = digitalRead(sw2);
  int button3 = digitalRead(sw3);
  
  if(button1 == 0){
    digitalWrite(r, 0);
    delay(100);
    stateButton = 1;
  }
  if(button2 == 0){
    digitalWrite(r, 0);
    delay(100);
    stateButton = 2;
  }
  if(button3 == 0){
    digitalWrite(r, 0);
    delay(100);
    stateButton = 3;
  }
  digitalWrite(r, 1); //1 คือ ไฟปิด
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
    long now[3];
    now[0] = millis();
    now[1] = millis();
    now[2] = millis();
    
    //LCD--show
    if (now[0] - lastMsg[0] > 500) { //mcu retrives sensor data 0.5s
            // DHT22
            float h = dht.readHumidity();
            float t = dht.readTemperature();
            if (isnan(h) || isnan(t) ) {
              Serial.println(F("Failed to read from DHT sensor!"));
            }
            //Light
            LDR_value=analogRead(Light_Pin);
            lux=int((250.000000/(ADC_value*LDR_value))-50.000000);
            
            now[0] = 0;
            lastMsg[0] = now[0];
    }
    if(stateButton == 1){
        printLocalTime();
    }
    if(stateButton == 2){
        lcd.setCursor(0, 1);
        lcd.print("Temp(*C): "+String(t));
        lcd.setCursor(0, 2);
        lcd.print("Humidity(%RH): "+String(h));
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
    
    if(stateButton == 3){
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("   ...Restart...    ");
      delay(1000);
      ESP.restart();
    }
    
    Serial.println("Temperature: " + (String)t + " °C ");
    Serial.println("Humidity: " + (String)h + " %RH");
    Serial.println("Lux = " + String(lux));

    //MQTT
    if (now[1] - lastMsg[1] > 1000) {
      now[1]=0;
      lastMsg[1] = now[1];
      snprintf (msg, 50, "%f",t);
      client.publish("Get_Temp", msg);
      snprintf (msg, 50, "%f",h);
      client.publish("Get_Humi", msg);
      snprintf (msg, 50, "%ld",lux);
      client.publish("Get_Light", msg);
  }
}

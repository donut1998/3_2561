#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <WiFiUdp.h>

#define sw1 12    //D6
#define relay1_sw_control 9  //SD2
#define relay2_MCU_control 10  //SD3
int buttonState = 1; 
int checkButton = 1;

bool switch_light_on = false;



// Update these with values suitable for your network.

const char* ssid = "Visitor";
const char* password = "security";
const char* mqtt_server = "163.50.57.25";
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];
int wait = 0;

// recieve data from MQTT
String light_status;
String light_control;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
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
  /*
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  
  Serial.println();
  Serial.println("-----------------------");
  if (String(topic) == "Control_Light") {
    Serial.print("Turn light ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(relay2_MCU_control, HIGH);
    }else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(relay2_MCU_control, LOW);
    }
  }
  */
  if (String(topic) == "light_status") {
    String data;
      for (int i = 0; i < length; i++) {
        //Serial.print((char)payload[i]);
        data += (char)payload[i];
      }  
       light_status = data;      
  }
  //Serial.println();
  if (String(topic) == "light_control") {
    String data;
      for (int i = 0; i < length; i++) {
        //Serial.print((char)payload[i]);
        data += (char)payload[i];
      }         
      light_control = data;
  }
  //Serial.println();
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
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("light_status");
      client.subscribe("light_control");
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
  setup_wifi();
  client.setServer(mqtt_server, mqttPort);
  client.setCallback(callback);
  Serial.begin(115200);
  pinMode(sw1,INPUT_PULLUP);
  pinMode(relay1_sw_control,OUTPUT);
  pinMode(relay2_MCU_control,OUTPUT);
  
  
}

void loop() {
  if(WiFi.status() == 0 || WiFi.status() == 1 || WiFi.status() == 4 || WiFi.status() == 5 || WiFi.status() == 6 ){
      setup_wifi();
      client.setServer(mqtt_server, mqttPort);
      client.setCallback(callback);
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  buttonState = digitalRead(sw1);
  if(checkButton != buttonState){
    if(buttonState == 1){
      Serial.println("Push");
      switch_light_on = !(switch_light_on);
    }
    checkButton = buttonState;
    delay(100);
  }
  if(switch_light_on){
    digitalWrite(relay1_sw_control,HIGH);
    //Serial.println("Turn light on");
  }else{
    digitalWrite(relay1_sw_control,LOW);
    //Serial.println("Turn light off");
  }
  /*
  if(light_status == "Light is on" && light_control == "on"){
    Serial.println("cmd on1");
    snprintf (msg, 50, "Light is on");
    client.publish("light_status", msg);
  }
  if(light_status == "Light is off" && light_control == "off"){
    Serial.println("cmd off1");
    snprintf (msg, 50, "Light is off");
    client.publish("light_status", msg);
  }
  
  if(light_status == "Light is on" && light_control == "off"){
    Serial.println("cmd off2");
    if(switch_light_on){
      digitalWrite(relay2_MCU_control,HIGH);
    }
    if(!switch_light_on){
      digitalWrite(relay2_MCU_control,LOW);
    }
    snprintf (msg, 50, "Light is off");
    client.publish("light_status", msg);
  }
  
  if(light_status == "Light is off" && light_control == "on"){
    Serial.println("cmd on2");
    if(switch_light_on){
      digitalWrite(relay2_MCU_control,LOW);
    }
    if(!switch_light_on){
      digitalWrite(relay2_MCU_control,HIGH);
    }
    snprintf (msg, 50, "Light is on");
    client.publish("light_status", msg);
  }
  Serial.println(light_status);
  */
  if(switch_light_on && light_control == "ON"){
    digitalWrite(relay2_MCU_control,HIGH);
    snprintf (msg, 50, "Light is on");
    client.publish("light_status", msg);
  }
  if(switch_light_on && light_control == "OFF"){
    digitalWrite(relay2_MCU_control,LOW);
    snprintf (msg, 50, "Light is off");
    client.publish("light_status", msg);
  }
  if(!switch_light_on && light_control == "ON"){
    digitalWrite(relay2_MCU_control,HIGH);
    snprintf (msg, 50, "Light is on");
    client.publish("light_status", msg);
  }
  if(!switch_light_on && light_control == "OFF"){
    digitalWrite(relay2_MCU_control,LOW);
    snprintf (msg, 50, "Light is off");
    client.publish("light_status", msg);
  }
  light_control = "";
  
  
}

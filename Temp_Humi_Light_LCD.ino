#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SimpleDHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 

// Update these with values suitable for your network.

const char* ssid = "Visitor";
const char* password = "security";
const char* mqtt_server = "163.50.57.110";

WiFiClient espClient;
PubSubClient client(espClient);
LiquidCrystal_I2C lcd(0x27, 20, 4);
long lastMsg = 0;
char msg[50];
//int value = 0;
int wait = 0;

//sensor
// dht22
int pinDHT22 = 12;
SimpleDHT22 dht22(pinDHT22);
float temperature = 0;
float humidity = 0;
//light
int lux;
float ADC_value = 0.0048828125;
float LDR_value;
int Light_Pin = A0;


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
  
  
}

void loop() {
  //Serial.println("wifi status = " + (String)WiFi.status());
  if(WiFi.status() == 0 || WiFi.status() == 1 || WiFi.status() == 4 || WiFi.status() == 5 || WiFi.status() == 6 ){
    ESP.restart();
  }
  
  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    
    // DHT22
    int err = SimpleDHTErrSuccess;
    if ((err = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("Read DHT22 failed, err="); Serial.println(err);delay(2000);
      return;
    }
    //Light
    LDR_value=analogRead(Light_Pin);
    lux=int((250.000000/(ADC_value*LDR_value))-50.000000);
//    if(lux == 2147483647){
//      lux = 5000;
//    }
//    Serial.println("Lux = "+ String(lux));
//    int mapLux = map(lux,-18,1200,0,3000);
//    if(mapLux<0){
//      mapLux=0;
//    }
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
    Serial.println("Temp = "+ String(temperature));
    Serial.println("Humi = " + String(humidity));
    Serial.println("Lux = " + String(lux));

    //String temp_humi_light = String(temperature)+","+String(humidity)+","+String(mapLux);
    snprintf (msg, 50, "%f",temperature);
    client.publish("Get_Temp", msg);
    //client.subscribe("Get_Temp");
    snprintf (msg, 50, "%f",humidity);
    client.publish("Get_Humi", msg);
    //client.subscribe("Get_Humi");
    snprintf (msg, 50, "%ld",lux);
    client.publish("Get_Light", msg);
    //client.subscribe("Get_Light");
    //client.subscribe("Get_Temp_Humi_Light");
  }
}

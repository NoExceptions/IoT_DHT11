
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>



const char* ssid = "<WIFI NETWORK>";
const char* password = "<WIFI PASSWORD>";

const char* clientId = "<CHOSE A NAME FOR YOUR DEVICE>";
const char* mqtt_user = "<BROKER USER>";
const char* mqtt_pwd = "<BROKER PASSWORD>";
const char* host = "<BROQUER IP/HOST NAME>"; //remembering the 

#define DHTPIN 5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

float t = 0.0;
float h = 0.0;

unsigned long previousMillis = 0; //ultima vez que o dht fez uma leitura

WiFiClient ethClient;
PubSubClient client(ethClient);


void callback(char* topic, byte* payload, unsigned int length) {


  if (strcmp(topic, "g/manual") == 0) {
    if (!strncmp((char*)payload, "on", length)) {
      Serial.println("Got signal to ON!");
      digitalWrite(25, HIGH);
      delay(200);
    } else if (!strncmp((char*)payload, "off", length)) {
      digitalWrite(25, LOW);
      delay(200);
    }
  }
}



void wifiConnect() {
  WiFi.begin(ssid,password); //WiFi.begin(ssid,password,0,myMac,true);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
    
    Serial.println(WiFi.status());
  }
  Serial.println("Connected to the WiFi network");
  Serial.println(WiFi.BSSIDstr());
}

void mqttConnect() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Missing WiFi..");
    Serial.println(WiFi.status());

    delay(1200);
    //wifiConnect();
  } else {
    if (!client.connected()) {
      Serial.println("1lets connect to the Broker..");
      if (client.connect(clientId, mqtt_user, mqtt_pwd)) {
        Serial.println("Connecting to Broker..");
        
        client.subscribe("g/manual");
      }
    }
  }
}



void setup() {
  pinMode(25, OUTPUT);
  Serial.begin(9600);

  WiFi.enableSTA(true);
  WiFi.enableAP(false);
  WiFi.setAutoConnect(true);
  WiFi.begin(ssid,password);


  client.setServer(host, 1883);
  client.setCallback(callback);

  dht.begin();



}

void loop() {
  if (!client.connected()) {
    Serial.println("MQTT reconection needed!");
    mqttConnect();
  } else {
    client.loop();
    delay(1200);
  }
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 5000) {
      float newT = dht.readTemperature();
      if (isnan(newT)) {
        Serial.println("Failed to read temperature from DHT sensor!");
      }
      else {
        char msg_out[20];
        dtostrf(newT, 2,2,msg_out);
        Serial.println(msg_out);

        client.publish("temperature/Periodic", msg_out);
        
        if( t != newT){
          client.publish("temperature/OnChange", msg_out);
          t = newT;
        }
        
        Serial.println(t);          
      }
    previousMillis = millis();
      
  }
  delay(1001);

}

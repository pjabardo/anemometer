
#include "anemometer.h"

Anemometer anem;


#include<WiFi.h>

const char *ssid = "wifi";
const char *password = "12345678";

const char *mqtt_server = "192.168.15.13";
const uint16_t mqtt_port = 1883;

WiFiClient esp_client;
PubSubClient client(esp_client);
const char *bname = "env";
//MQTTAnem mqtt_anem(bname, 0, 0);

void callback(char *topic, byte* message, unsigned int length){
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Tentando conectar com o Broker do MQTT ...");
    // Attempt to connect
    if (client.connect("JAnem")) {
      Serial.println("conectado");
      // Subscribe
      //client.subscribe("esp32/output");
      // Publish retained parameters
      //mqtt_anem.publish_params();

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup(){
  Serial.begin(9600);
  
  //WiFi.begin(ssid, password);
  Serial.println("\nConnecting");
  //while (WiFi.status() != WL_CONNECTED){
  //  Serial.print(".");
  //  delay(200);
 // }
  Serial.println("\nConnected to the WiFi networkd");
  Serial.print("\nLocal ESP32 IP: ");
  //Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  anem.setup_anemometer();
  anem.setup_temperature();
  Serial.println("SETUP REALIZADO");
  //mqtt_anem.initialize(&anem, &client);
  //reconnect();
}



void loop(){
  if (!client.connected()) {
    //reconnect();
    Serial.println("TENTANDO RECONECTAR");
  }
  Serial.println(WiFi.localIP());

  Serial.print("P = ");
  Serial.println(anem.read_pressure());
  delay(500);

  //mqtt_anem.loop();
  

}


#include<WiFi.h>

#include "mqtt_anem.h"


Anemometer anem;

const char *ssid = "rede-wifi";
const char *password = "senhadarede";

const char *mqtt_server = "192.168.0.100";
const uint16_t mqtt_port = 1883;

WiFiClient esp_client;
PubSubClient client(esp_client);
const char *bname = "env";
MQTTAnem mqtt_anem(bname, &anem, &client);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Tentando conectar com o Broker do MQTT ...");
    // Attempt to connect
    if (client.connect("JAnem")) {
      Serial.println("conectado");

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
  
  WiFi.begin(ssid, password);
  
  Serial.println("\nConnecting");
  Serial.println("\nConnecting");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }  
  Serial.println("\nConnected to the WiFi network!");

  Serial.print("\nLocal ESP32 IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  
  
  anem.setup_anemometer();
  anem.setup_temperature();

  reconnect();
  mqtt_anem.publish_params();

}



void loop(){

  if (!client.connected()) {
    reconnect();
    Serial.println(WiFi.localIP());
  }

  mqtt_anem.loop();


}


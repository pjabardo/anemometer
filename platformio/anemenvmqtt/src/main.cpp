
#include "anemometer.h"
#include <PubSubClient.h>

Anemometer anem;


#include<WiFi.h>

const char *ssid = "wifi";
const char *password = "12345678";

const char *mqtt_server = "192.168.0.100";
const uint16_t mqtt_port = 1883;

WiFiClient esp_client;
PubSubClient client(esp_client);

//AnemometerComm<WiFiClient> comm(0, &anem);
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


void setup(){
  Serial.begin(9600);
  
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConnected to the WiFi networkd");
  Serial.print("\nLocal ESP32 IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  anem.setup_anemometer();
  anem.setup_temperature();

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
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop(){
  if (!client.connected()) {
    reconnect();
  }
  uint8_t ntemp = anem.numtemp();
  float x;

  char buf[14]; // Buffer para armazenar números
  client.loop();
  
  // Read pressure
  x = anem.read_pressure();
  dtostrf(x, 10, 2, buf);
  Serial.print("Pa = ");
  Serial.println(x);
  client.publish("env/patm", buf);

  // Ler umidade
  x = anem.read_humidity();
  dtostrf(x, 10, 3, buf);
  Serial.print("H = ");
  Serial.println(x);
  client.publish("env/H", buf);

  // Ler temperatura do sensor de umidade
  x = anem.read_dht_temperature();
  dtostrf(x, 10, 3, buf);
  Serial.print("Th = ");
  Serial.println(x);
  client.publish("env/TH", buf);

  // Ler os sensores de temperatura
  x = anem.read_temperature(0);
  dtostrf(x, 10, 3, buf);
  Serial.print("T0 = ");
  Serial.println(x);
  client.publish("env/T0", buf);

  // Ler os sensores de temperatura
  x = anem.read_temperature(1);
  dtostrf(x, 10, 3, buf);
  Serial.print("T1 = ");
  Serial.println(x);
  client.publish("env/T1", buf);

  // Ler os sensores de temperatura
  x = anem.read_temperature(0);
  dtostrf(x, 10, 3, buf);
  Serial.print("T2 = ");
  Serial.println(x);
  client.publish("env/T2", buf);

}


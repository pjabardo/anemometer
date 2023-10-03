#include "anemometer.h"

Anemometer anem;
//#define _USE_SERIAL_
#define _USE_WIFI_
//#define _USE_BLUETOOTH_

#ifdef _USE_SERIAL_
#undef _USE_WIFIL_
#undef _USE_BLUETOOTH_

AnemometerComm<HardwareSerial> comm(&Serial, &anem);

void setup(){
  Serial.begin(115200);
  anem.setup_anemometer();
}



void loop(){
  if (comm.available()){
    comm.repl();
  }
}

#endif //_USE_SERIAL_


#ifdef _USE_WIFI_
#undef _USE_SERIAL_
#undef _USE_BLUETOOTH_

#include<WiFi.h>
const char *ssid = "tunel";
const char *password = "gvento123";

WiFiServer server(9525);

AnemometerComm<WiFiClient> comm(0, &anem);

void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }

  Serial.println("\nConnected to the WiFi networkd");
  Serial.print("\nLocal ESP32 IP: ");
  Serial.println(WiFi.localIP());
  
  server.begin();
  
  anem.setup_anemometer();
}



void loop(){
  WiFiClient client = server.available();
  delay(500);


  if (client){
    client.setTimeout(1);
    Serial.println("Client connected!");
    while (client.connected()){
      comm.set_comm(&client);

      if (comm.available()){
        comm.repl();
      }
    }
  }
  //Serial.println("Finished loop!");
}



#endif

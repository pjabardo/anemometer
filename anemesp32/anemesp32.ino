
//#define USE_BT
//#define USE_WIFI
#define USE_SERIAL
#define ONLY_SERIAL

#ifdef USE_BT
#include "BluetoothSerial.h"
#endif

#ifdef USE_WIFI
#include <WiFi.h>
#endif

// Get 1-wire Library here: http://www.pjrc.com/teensy/td_libs_OneWire.html
#include <OneWire.h>


//Get DallasTemperature Library here:  http://milesburton.com/Main_Page?title=Dallas_Temperature_Control_Library
#include <DallasTemperature.h>

#include <DHT.h>

#include <Adafruit_BMP280.h>
#include <Adafruit_ADS1X15.h>

#include "anemometer.h"

Adafruit_BMP280 bmp; // use I2C interface
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();


#ifdef USE_BT
BluetoothSerial SerialBT;
#endif

#ifdef USE_WIFI
const char* ssid = "durruti";
const char* password = "ginzburglanda";
#endif

Adafruit_ADS1115 ads;

#define TEMPPIN 23  // DS18B20 pin
#define DHTPIN 33   // DHT22 pin

#define DHTTYPE DHT22

/*-----( Declare objects )-----*/
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(TEMPPIN);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

/*-----( Declare Variables )-----*/
// Assign the addresses of your 1-Wire temp sensors.
// See the tutorial on how to obtain these addresses:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html

DeviceAddress probes[] ={ { 0x28, 0x8D, 0xFA, 0x79, 0x97, 0x09, 0x03, 0x9C }, 
                      { 0x28, 0xFF, 0x8E, 0xDE, 0x82, 0x15, 0x02, 0xB7 },  
                      { 0x28, 0xFF, 0xC1, 0xF8, 0x82, 0x15, 0x02, 0x48 }, 
                      { 0x28, 0xFF, 0x7E, 0x0A, 0x82, 0x15, 0x03, 0x40 }, 
                      { 0x28, 0xFF, 0xAF, 0xF3, 0x82, 0x15, 0x02, 0x6F }};

#ifdef USE_WIFI
WiFiServer server(80);

void setup_wifi(const char *ssid, const char *password)
{

  WiFi.begin(ssid, password);

#ifdef USE_SERIAL
  Serial.println("\nConnecting");
#endif

  while(WiFi.status() != WL_CONNECTED){
#ifdef USE_SERIAL
    Serial.print(".");
    delay(100);
#endif
  }

#ifdef USE_SERIAL
  Serial.println("\nConnected to the WiFi network");
  Serial.println("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
#endif
  
}


#endif


const int NPROBES = 5;

DHT dht(DHTPIN, DHTTYPE);

void setup_ads(int addr=0x4A){
  ads.setGain(GAIN_TWO);

  if (!ads.begin(addr)) {
    return;
  }

}
void setup_bmp(int addr=0x76){
  if (!bmp.begin(addr)){

    return;
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void setup_dht(){

  dht.begin();
 
}

Anemometer myanem(&Serial, &bmp, &ads, &sensors, &dht);

void setup_temp(){
  sensors.begin();
  for (int i = 0; i < NPROBES; ++i)
    sensors.setResolution(probes[i], 12);

}
void setup()   /****** SETUP: RUNS ONCE ******/
{
#ifdef USE_SERIAL
Serial.begin(9600);
#endif
  
  // start serial port to show results
#ifdef USE_BT
  SerialBT.begin("AnemPJ");
#endif

#ifdef USE_WIFI
setup_wifi(ssid, password);
#endif


  setup_bmp(0x76);
  setup_temp();
  setup_dht();
  setup_ads(0x4A);
  Serial.println(sensors.getDeviceCount());

}//--(end setup )---

void clear_buffer(int ms=50){

#ifdef USE_BT
  while(SerialBT.available() > 0){
    delay(ms);
    SerialBT.readString();
  }
#endif

#ifdef ONLY_SERIAL
  while(Serial.available() > 0){
    delay(ms);
    Serial.readString();
  }
#endif  

}

#ifdef ONLY_SERIAL

void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  delay(50);
  char cmd;
  char var;
  char var2;
  bool foundvar = false;  
  int16_t adcx;
  float voltx;
  
  String s;  

  if (Serial.available() > 0){
    cmd = Serial.read();
    if (cmd == '%'){
      Serial.flush();
      clear_buffer(50);
      return;
    }
    if (cmd != '*'){
      
      Serial.print("ERR - Unknown command -->");
      Serial.println(cmd);
      Serial.flush();
      clear_buffer(50);
      return;
    }
    delay(50);
    foundvar = false;
    for (int i = 0; i < 10; ++i){
      if (Serial.available() <= 0){
        delay(50);
      }else{
        foundvar = true;
        break;
      }
    }
    if (!foundvar){
      Serial.println("ERR - No variable found");
      Serial.flush();
      clear_buffer(50);
      return;
    }
    
    var = Serial.read();
    if (var != 'P' && var != 'H' && var != 'T' && var != 'S' && var != 'A'){
      Serial.print("ERR - Unknown variable -->");
      Serial.println(var);
      Serial.flush();
      clear_buffer(50);
      return;
    }

    // Check variable specifier
    delay(50);
    foundvar = false;
    for (int i = 0; i < 10; ++i){
      if (Serial.available() <= 0){
        delay(50);
      }else{
        foundvar = true;
        break;
      }
    }
    if (!foundvar){
      Serial.println("ERR - No sub-variable found");
      Serial.flush();
      clear_buffer(50);
      return;
    }

    var2 = Serial.read();

    if (var == 'P'){
      if (var2 == 'P'){
        Serial.println(bmp.readPressure());
        Serial.flush();
      }else if (var2=='T'){
        Serial.println(bmp.readTemperature());     
        Serial.flush();
      }else{
        Serial.print("ERR - Variable does not exist. Only H or T available -->");
        Serial.println(var2);
        Serial.flush();
      }
    } else if (var == 'H'){
      if (var2 == 'H'){
        Serial.println(dht.readHumidity());
        Serial.flush();
      }else if (var2=='T'){
        Serial.println(dht.readTemperature());     
        Serial.flush();
      }else{
        Serial.print("ERR - Variable does not exist. Only T or P available -->");
        Serial.println(var2);
        Serial.flush();
      }
      
    } else if (var == 'T'){
      if (var2 < '1' || var2 > '5'){
        Serial.print("ERR - Variable does not exist. 1-5 possible -->");
        Serial.println(var2);
        Serial.flush();
      }else{
        char idx = var2 - '1';
        //sensors.requestTemperatures();  
        sensors.requestTemperaturesByIndex(idx);
        Serial.println(sensors.getTempCByIndex(idx));
        Serial.flush();
      }
    }else if (var == 'A'){ // Analog input
      if (var2 < '0' || var2 > '3'){
        Serial.print("ERR - Analog input does not exist. 0-3 possible -->");
        Serial.println(var2);
        Serial.flush();
      }else{
        char idx = var2 - '0';
        adcx = ads.readADC_SingleEnded(idx);
        voltx = ads.computeVolts(adcx);
        Serial.println(voltx);
        Serial.flush();
        
      }
      
    }else if (var == 'S'){  // Status
      Serial.println("OK");
      Serial.flush();
    }
  }

  Serial.flush();

  clear_buffer(); 
}
#endif

#ifdef USE_BT
void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  delay(50);
  char cmd;
  char var;
  char var2;
  bool foundvar = false;  
  int16_t adcx;
  float voltx;
  
  String s;  

  if (SerialBT.available() > 0){
    cmd = SerialBT.read();
    if (cmd == '%'){
      SerialBT.flush();
      clear_buffer(50);
      return;
    }
    if (cmd != '*'){
      
      SerialBT.print("ERR - Unknown command -->");
      SerialBT.println(cmd);
      SerialBT.flush();
      clear_buffer(50);
      return;
    }
    delay(50);
    foundvar = false;
    for (int i = 0; i < 10; ++i){
      if (SerialBT.available() <= 0){
        delay(50);
      }else{
        foundvar = true;
        break;
      }
    }
    if (!foundvar){
      SerialBT.println("ERR - No variable found");
      SerialBT.flush();
      clear_buffer(50);
      return;
    }
    
    var = SerialBT.read();
    if (var != 'P' && var != 'H' && var != 'T' && var != 'S' && var != 'A'){
      SerialBT.print("ERR - Unknown variable -->");
      SerialBT.println(var);
      SerialBT.flush();
      clear_buffer(50);
      return;
    }

    // Check variable specifier
    delay(50);
    foundvar = false;
    for (int i = 0; i < 10; ++i){
      if (SerialBT.available() <= 0){
        delay(50);
      }else{
        foundvar = true;
        break;
      }
    }
    if (!foundvar){
      SerialBT.println("ERR - No sub-variable found");
      SerialBT.flush();
      clear_buffer(50);
      return;
    }

    var2 = SerialBT.read();

    if (var == 'P'){
      if (var2 == 'P'){
        SerialBT.println(bmp.readPressure());
        SerialBT.flush();
      }else if (var2=='T'){
        SerialBT.println(bmp.readTemperature());     
        SerialBT.flush();
      }else{
        SerialBT.print("ERR - Variable does not exist. Only H or T available -->");
        SerialBT.println(var2);
        SerialBT.flush();
      }
    } else if (var == 'H'){
      if (var2 == 'H'){
        SerialBT.println(dht.readHumidity());
        SerialBT.flush();
      }else if (var2=='T'){
        SerialBT.println(dht.readTemperature());     
        SerialBT.flush();
      }else{
        SerialBT.print("ERR - Variable does not exist. Only T or P available -->");
        SerialBT.println(var2);
        SerialBT.flush();
      }
      
    } else if (var == 'T'){
      if (var2 < '1' || var2 > '5'){
        SerialBT.print("ERR - Variable does not exist. 1-5 possible -->");
        SerialBT.println(var2);
        SerialBT.flush();
      }else{
        char idx = var2 - '1';
        sensors.requestTemperatures();  
        SerialBT.println(sensors.getTempC(probes[idx]));
        SerialBT.flush();
      }
    }else if (var == 'A'){ // Analog input
      if (var2 < '0' || var2 > '3'){
        SerialBT.print("ERR - Analog input does not exist. 0-3 possible -->");
        SerialBT.println(var2);
        SerialBT.flush();
      }else{
        char idx = var2 - '0';
        adcx = ads.readADC_SingleEnded(idx);
        voltx = ads.computeVolts(adcx);
        SerialBT.println(voltx);
        SerialBT.flush();
        
      }
      
    }else if (var == 'S'){  // Status
      SerialBT.println("OK");
      SerialBT.flush();
    }
  }

  SerialBT.flush();

  clear_buffer(); 

}
#endif 


#ifdef USE_WIFI

void clear_wifi_buffer(WiFiClient client, int ms=50)
{

  
}
void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  //Serial.println("Esperando...");
  //delay(20);

  WiFiClient client = server.available();  // Listen for incoming clients
  if (client) {
    Serial.println("CLiente tentando conectar");
    char cmd;
    char var;
    char var2;
    bool foundvar = false;  
    int16_t adcx;
    float voltx;
    
    String s;  
    while (client.connected()){
      Serial.println("Cliente connected!");
      if (client.available() > 0){
        cmd = client.read();
        if (cmd == '%'){
          client.flush();
          clear_wifi_buffer(client, 50);
          return;
        }
        if (cmd != '*'){
          
          client.print("ERR - Unknown command -->");
          client.println(cmd);
          client.flush();
          clear_wifi_buffer(50);
          return;
        }
        delay(50);
        foundvar = false;
        for (int i = 0; i < 10; ++i){
          if (client.available() <= 0){
            delay(50);
          }else{
            foundvar = true;
            break;
          }
        }
        if (!foundvar){
          client.println("ERR - No variable found");
          client.flush();
          clear_wifi_buffer(client, 50);
          return;
        }
        
        var = client.read();
        if (var != 'P' && var != 'H' && var != 'T' && var != 'S' && var != 'A'){
          client.print("ERR - Unknown variable -->");
          client.println(var);
          client.flush();
          clear_wifi_buffer(client, 50);
          return;
        }
    
        // Check variable specifier
        delay(50);
        foundvar = false;
        for (int i = 0; i < 10; ++i){
          if (client.available() <= 0){
            delay(50);
          }else{
            foundvar = true;
            break;
          }
        }
        if (!foundvar){
          client.println("ERR - No sub-variable found");
          client.flush();
          clear_wifi_buffer(client, 50);
          return;
        }
    
        var2 = client.read();
    
        if (var == 'P'){
          if (var2 == 'P'){
            client.println(bmp.readPressure());
            client.flush();
          }else if (var2=='T'){
            client.println(bmp.readTemperature());     
            client.flush();
          }else{
            client.print("ERR - Variable does not exist. Only H or T available -->");
            client.println(var2);
            client.flush();
          }
        } else if (var == 'H'){
          if (var2 == 'H'){
            client.println(dht.readHumidity());
            client.flush();
          }else if (var2=='T'){
            client.println(dht.readTemperature());     
            client.flush();
          }else{
            client.print("ERR - Variable does not exist. Only T or P available -->");
            client.println(var2);
            client.flush();
          }
          
        } else if (var == 'T'){
          if (var2 < '1' || var2 > '5'){
            client.print("ERR - Variable does not exist. 1-5 possible -->");
            client.println(var2);
            client.flush();
          }else{
            char idx = var2 - '1';
            sensors.requestTemperatures();  
            client.println(sensors.getTempC(probes[idx]));
            client.flush();
          }
        }else if (var == 'A'){ // Analog input
          if (var2 < '0' || var2 > '3'){
            client.print("ERR - Analog input does not exist. 0-3 possible -->");
            client.println(var2);
            client.flush();
          }else{
            char idx = var2 - '0';
            adcx = ads.readADC_SingleEnded(idx);
            voltx = ads.computeVolts(adcx);
            client.println(voltx);
            client.flush();
            
          }
          
        }else if (var == 'S'){  // Status
          client.println("OK");
          client.flush();
        }
      }
    
      client.flush();
    
      clear_wifi_buffer(client, 50); 
    }
  }
}
#endif 
  

//*********( THE END )***********

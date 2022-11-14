// Get 1-wire Library here: http://www.pjrc.com/teensy/td_libs_OneWire.html
#include <OneWire.h>

//Get DallasTemperature Library here:  http://milesburton.com/Main_Page?title=Dallas_Temperature_Control_Library
#include <DallasTemperature.h>

#include <DHT.h>

#include <Adafruit_BMP280.h>
#include <Adafruit_ADS1X15.h>

Adafruit_BMP280 bmp; // use I2C interface
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

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

void setup_temp(){
  sensors.begin();
  for (int i = 0; i < NPROBES; ++i)
    sensors.setResolution(probes[i], 12);

}
void setup()   /****** SETUP: RUNS ONCE ******/
{
  // start serial port to show results
  Serial.begin(9600);
  
  setup_bmp(0x76);
  setup_temp();
  setup_dht();
  setup_ads(0x4A);
  

}//--(end setup )---

void clear_buffer(int ms=50){

  while(Serial.available() > 0){
    delay(ms);
    Serial.readString();
  }
  
}


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
        sensors.requestTemperatures();  
        Serial.println(sensors.getTempC(probes[idx]));
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
  

//*********( THE END )***********

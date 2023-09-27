

#ifndef __ANEMOMETER_H__
#define __ANEMOMETER_H__

#include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_ADS1X15.h>
#include <DHT.h>

const uint8_t MAXTCHANS = 8;
class Anemometer
{
protected:
  uint8_t _bmp_addr;
  uint8_t _dht_pin;
  uint8_t _dht_type;
  uint8_t _temp_pin;
  uint8_t _daq_addr;
  adsGain_t _gain;
  uint8_t _temp_resolution;

  int8_t _ich[4]; // AI channels that should be read
  uint8_t _nch; // Number of AI channels that should be read
  
  OneWire _one_wire;
  Adafruit_BMP280 _bmp;
  Adafruit_ADS1115 _daq;
  DallasTemperature _temp;
  DHT _dht;
  
public:
  Anemometer(uint8_t bmp_addr=0x76, uint8_t dht_pin=33, uint8_t dht_type=DHT22, 
            uint8_t temp_pin=23, uint8_t daq_addr=0x4A, adsGain_t gain=GAIN_TWO,
            uint8_t temp_resolution=12):
            _bmp_addr(bmp_addr), _dht_pin(dht_pin), _dht_type(dht_type), 
            _temp_pin(temp_pin), _daq_addr(daq_addr), _gain(gain), 
            _temp_resolution(temp_resolution), _bmp(), _dht(dht_pin, dht_type), 
            _one_wire(temp_pin), _daq(), _temp(&_one_wire){}
   
  void setup_anemometer();
  void setup_temperature();
  
  float read_dht_temperature(bool S = false, bool force = false);
  float read_humidity(bool force = false);

  float read_bmp_temperature();
  float read_pressure();

  float read_temperature(uint8_t idx);
  int16_t read_aichan(uint8_t idx);
   
  void read_frame(int16_t *adcx);

  uint8_t ai_chans(int8_t i0=0, int8_t i1=-1, int8_t i2=-1, int8_t i3=-1);

  uint8_t numchans(){ return _nch; }
  int8_t* chanidx(){ return _ich; }
};

  
template <typename Comm>
class AnemometerComm
{
protected:
  Comm *_comm;  // Handles communication with computer
  Anemometer *_anem; // Handles data acquisition
  uint16_t _fps;  // Number of samples that should be read
  uint16_t _avg; // Number of samples that should be averaged
  int32_t _frame[4]; // Place to store the samples read
  char _buffer[32]; // Buffer
  DeviceAddress _taddr[MAXTCHANS];   // Stores DS18B20 addresses
  uint8_t _ntchans; // Number of temperature sensors actually present 
  uint8_t _tchans[MAXTCHANS]; // Temperature sensors
  
public:
  AnemometerComm(Comm *comm, Anemometer *anem):
    _comm(comm), _anem(anem){_fps=1; _avg=1;}
  
  void set_comm(Comm *comm=0){
    _comm = comm;
  }
  
  bool available(){
    return _comm->available();
  }
  void set_avg(uint16_t avg){
    if (avg > 2000)
      avg = 1000;
    _avg = avg;
  }

  void set_fps(uint16_t fps){
    _fps = fps;
  }

  int parse_setvar(String cmd){
    int x;
    cmd.trim();
    _comm->println(cmd);
    int idx = cmd.indexOf(' ');
    if (idx < 0){
      return 1;
    }
    String var = cmd.substring(0, idx);
    String val = cmd.substring(idx+1);
    
    val.trim();
    if (var=="FPS"){
      x = val.toInt();
      if (x == 0){
        return 1;
      }else{
        set_fps(x);
      }
    }else if(var=="AVG"){
      x = val.toInt();
      if (x == 0){
        return 1;
      }else{
        set_avg(x);
      }
      
    }else{
      return 1;
    }

    return 0;
  }

  int readcmd(String cmd){
    cmd.trim();
    _comm->println(cmd);
    if (cmd=="P"){
      _comm->println("1");
      _comm->println(_anem->read_pressure());
      return 0;
    }else if (cmd=="PT"){
      _comm->println("1");
      _comm->println(_anem->read_bmp_temperature());
      return 0;
    }else if (cmd=="H"){
      _comm->println("1");
      _comm->println(_anem->read_humidity());
      return 0;
    }else if (cmd=="HT"){
      _comm->println("1");
      _comm->println(_anem->read_dht_temperature());
      return 0;
    }else if (cmd[0]=='T'){
      uint8_t itemp = cmd[1] - '0';
      if (itemp < 3){
        _comm->println("1");
        _comm->println(_anem->read_temperature(itemp));
        return 0;
      }else{
        return 1;
      }
    }else if (cmd[0] == 'A' && cmd[1] == 'I'){
      uint8_t ich = cmd[2] - '0'; 
      if (ich < 4){
        _comm->println("1");
        _comm->println(_anem->read_aichan(ich));
      }
        
    }
  }
  
  void envconds(){
    _comm->println(5);
    _comm->println(_anem->read_pressure());
    _comm->println(_anem->read_humidity());
    for (int i = 0; i < 3; ++i)
      _comm->println(_anem->read_temperature(i));
    _comm->println("OK");
  }

  void scan(){
    int16_t fr[4];
    uint8_t nch = _anem->numchans();
    
    _comm->println(_fps);
    _comm->println(nch);
    
    unsigned long t1 = millis();
    for (int i = 0; i < _fps; ++i){
      for (int k = 0; k < nch; ++k){
        _frame[k] = 0;
      }
      for (int j = 0; j < _avg; ++j){
        _anem->read_frame(fr);
        for (int k = 0; k < nch; ++k){
          _frame[k] += fr[k];
        }
      }
      for (int k = 0; k < nch; ++k){
        _comm->println(_frame[k] / _avg);
      }
    }
    _comm->println( (millis() - t1) / 1000.0);
    _comm->println("OK");
  }
  
  void repl(){
    String line;
    // Let's read a lines of input
    line = _comm->readStringUntil('\n');
    line.trim();
    line.toUpperCase();
    if (line.startsWith("SET")){
      if (parse_setvar(line.substring(3))){
        _comm->println("ERR");
      }else{
        _comm->println("OK");
      }
    }else if (line.startsWith("SCAN")){
      scan();     
    }else if (line.startsWith("READ")){
      if (readcmd(line.substring(5))){
        _comm->println("ERR");     
      }else{
        _comm->println("OK");
      }
    }else if (line.startsWith("ENV")){
      envconds();
    }else if (line=="STATUS"){
      _comm->println("OK");
    }else{
      _comm->println("ERR");
    }
  }
       
  
};





#endif __ANEMOMETER_H__

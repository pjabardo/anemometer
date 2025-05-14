

#ifndef __ANEMOMETER_H__
#define __ANEMOMETER_H__

#include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_ADS1X15.h>
#include <DHT.h>


#define ONBOARD_LED 2

const uint8_t MAXTCHANS = 8;
class Anemometer
{
protected:
  uint8_t _bmp_addr;
  uint8_t _dht_pin;
  uint8_t _dht_type;
  uint8_t _temp_pin;
  uint8_t _daq_addr;

  uint8_t _temp_resolution;
  DeviceAddress _taddr[MAXTCHANS];   // Stores DS18B20 addresses
  uint8_t _ntaddr; // Number of DS18b20 available in the line.
  
  adsGain_t _gain;
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
  uint8_t load_temp_sensors();
  
  
  float read_dht_temperature(bool S = false, bool force = false);
  float read_humidity(bool force = false);

  float read_bmp_temperature();
  float read_pressure();

  float read_temperature(uint8_t idx);
  int16_t read_aichan(uint8_t idx);
   
  void read_frame(int16_t *adcx);

  int8_t ai_chans(int8_t i0=0, int8_t i1=-1, int8_t i2=-1, int8_t i3=-1);

  int8_t numchans(){ return _nch; }
  int8_t* chanidx(){ return _ich; }
  int  chanidx(uint8_t i);
  void clear_aichans();
  int add_aichan(uint8_t idx);
  
  uint8_t tempres(){ return _temp_resolution; }
  uint8_t *tempaddr(uint8_t itemp);
  uint8_t numtemp(){ return _ntaddr; }
  void set_temp_res(uint8_t tres);
  
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
      return -101;
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
    }else if (var=="TRES"){
      x = val.toInt();
      _anem->set_temp_res(x);
    }else{
      return -102;
    }

    return 0;
  }

  int parse_list_var(String cmd)
  {
    cmd.trim();
    _comm->println(cmd);
    if (cmd=="AI"){
      _comm->println(2);
      _comm->println(_avg);
      _comm->println(_fps);
      return 0;
    }else if (cmd=="TRES"){
      _comm->println(1);
      _comm->println(_anem->tempres());
      return 0;
    }else if (cmd == "AI"){
      _comm->println(cmd);
      _comm->println(_anem->numchans());
      for (uint8_t i = 0; i < _anem->numchans(); ++i){
        _comm->println(_anem->chanidx(i));
      }
      return 0;
    }else if (cmd == "TEMP"){
      
    }else{
      return -301;
    }
    
    return -301;
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
      if (itemp < _anem->numtemp() && itemp >= 0){
        _comm->println("1");
        _comm->println(_anem->read_temperature(itemp));
        return 0;
      }else{
        return -401;
      }
    }else if (cmd[0] == 'A' && cmd[1] == 'I'){
      uint8_t ich = cmd[2] - '0'; 
      int16_t aival;
      if (ich < 4){
        aival = _anem->read_aichan(ich);
        _comm->println("1");
        _comm->println(aival);
        return 0;
      }else{
        return -402;
      }
      
        
    }
    return -403;
  }
  
  void envconds(){
    _comm->println(5);
    _comm->println(_anem->read_pressure());
    _comm->println(_anem->read_humidity());
    for (uint8_t i = 0; i < _anem->numtemp(); ++i)
      _comm->println(_anem->read_temperature(i));
    _comm->println("OK");
  }

  int scan(String cmd){

    uint16_t fps, avg;
    int idx;
    cmd.trim();
    if (cmd.length() == 0){
      fps = _fps;
      avg = _avg;
    }else{
      idx = cmd.indexOf(' ');
      if (idx < 0){
          fps = cmd.toInt();
          if (fps <= 0){
            return -401;
          }
          avg = _avg;
      }else{
        fps = cmd.substring(0, idx).toInt();
        if (fps<=0){
          return -401;
        }
        avg = cmd.substring(idx+1).toInt();
        if (avg <= 0){
          return -402;
        }
        
      }
    }
    _comm->println("START");
    int16_t fr[4];
    uint8_t nch = _anem->numchans();
    
    _comm->println(fps);
    _comm->println(nch);
    //Serial.println("LIGANDO LED");
    //digitalWrite(ONBOARD_LED, HIGH);
    
    unsigned long t1 = micros();
    for (int i = 0; i < fps; ++i){
      for (int k = 0; k < nch; ++k){
        _frame[k] = 0;
      }
      for (int j = 0; j < avg; ++j){
        _anem->read_frame(fr);
        for (int k = 0; k < nch; ++k){
          _frame[k] += fr[k];
        }
      }
      _comm->print(i);
      _comm->print(" ");
      _comm->print( (micros() - t1) / 1e6);
      for (int k = 0; k < nch; ++k){
        _comm->print(" ");
        _comm->print(_frame[k] / avg);
      }
      _comm->println("");
      if (_comm->available()){
        //Serial.println("DESLIGANDO LED");
       // digitalWrite(ONBOARD_LED, LOW);
        char ch = _comm->read();
        if (ch == '!'){
          _comm->readStringUntil('\n');
          delay(50);
        }else{
          delay(50);
          return -410;
        }
        return 0;
      }
    }
  //Serial.println("DESLIGANDO LED");
  //digitalWrite(ONBOARD_LED, LOW);
    
  return 0;    
  }
  
  void repl(){
    String line;
    String cmd;
    int err;
    // Let's read a lines of input
    line = _comm->readStringUntil('\n');
    line.trim();
    line.toUpperCase();
    
    if (line.startsWith("SET")){
      cmd = line.substring(4);
      err = parse_setvar(cmd);
      if (err){
        _comm->println("ERR");
        _comm->println(err);
        _comm->print(cmd);
      }else{
        _comm->println("OK");
      }
    }else if (line.startsWith("LIST")){
      cmd = line.substring(5);
      err = parse_list_var(cmd);
      if (err){
        _comm->println("ERR");
        _comm->println(err);
        _comm->println(cmd);
      }else{
        _comm->println("OK");
      }
    }else if (line.startsWith("SCAN")){
      err = scan(line.substring(5));     
      if (err){
        _comm->println("ERR");
        _comm->println(err);
        _comm->println(line);
      }else{
        _comm->println("OK");
      }
    }else if (line.startsWith("READ")){
      cmd = line.substring(5);
      err = readcmd(cmd);
      if (err){
        _comm->println("ERR");     
        _comm->println(err);
        _comm->println(cmd);
      }else{
        _comm->println("OK");
      }
    }else if (line=="STATUS"){
      _comm->println("OK");
    }else if (line.startsWith("CHANS")){
      _comm->println(line);
      _comm->println(_anem->numchans());
      for (uint8_t i = 0; i < _anem->numchans(); ++i){
          _comm->println(_anem->chanidx(i));      
      }
      _comm->println("OK");
    }else if (line.startsWith("NUMCHANS")){
      _comm->println(line);
      _comm->println(_anem->numchans());
      _comm->println("OK");
    }else if (line.startsWith("CLEARCHANS")){
      _anem->clear_aichans();
      _comm->println("OK");
    }else if (line.startsWith("ADDCHAN")){
      err = _anem->add_aichan(line.substring(8).toInt());
      if (err){
        _comm->println("ERR");
        _comm->println(err);
        _comm->println(line);
      }else{
        _comm->println("OK");
      }
    }else if(line.startsWith("LOADTEMP")){
      _comm->println(line);
      _comm->println(_anem->load_temp_sensors());
      _comm->println("OK");
    }else if (line.startsWith("NUMTEMP")){
      _comm->println(line);
      _comm->println(_anem->numtemp());
      _comm->println("OK");
    }else if (line.startsWith("TEMPCHANS")){
      _comm->println(line);
      _comm->println(_anem->numtemp());
      for (uint8_t i = 0; i < _anem->numtemp(); ++i){
        uint8_t *addr = _anem->tempaddr(i);
        if (addr){
          for (uint8_t k = 0; k < 8; ++k){
            _comm->print(addr[k]);
            _comm->print(" ");
          }
        }else{
          _comm->print("0 0 0 0 0 0 0 0 ");
        }
        _comm->println("");
      }
      _comm->println("OK");
    }else{
      _comm->println("ERR");
      _comm->println(-1);
      _comm->println(line);
    }
   }
    
};
       
  





#endif //__ANEMOMETER_H__

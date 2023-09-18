

#ifndef __ANEMOMETER_H__
#define __ANEMOMETER_H__

#include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_ADS1X15.h>
#include <DHT.h>

class Anemometer
{
protected:
  uint8_t bmp_addr_;
  uint8_t dht_pin_;
  uint8_t dht_type_;
  uint8_t temp_pin_;
  uint8_t daq_addr_;
  adsGain_t gain_;
  uint8_t temp_resolution_;

  int8_t ich_[4]; // AI channels that should be read
  uint8_t nch_; // Number of AI channels that should be read
  
  OneWire one_wire_;
  Adafruit_BMP280 bmp_;
  Adafruit_ADS1115 daq_;
  DallasTemperature temp_;
  DHT dht_;
  
public:
  Anemometer(uint8_t bmp_addr=0x76, uint8_t dht_pin=33, uint8_t dht_type=DHT22, 
            uint8_t temp_pin=23, uint8_t daq_addr=0x4A, adsGain_t gain=GAIN_TWO,
            uint8_t temp_resolution=12):
            bmp_addr_(bmp_addr), dht_pin_(dht_pin), dht_type_(dht_type), 
            temp_pin_(temp_pin), daq_addr_(daq_addr), gain_(gain), 
            temp_resolution_(temp_resolution), bmp_(), dht_(dht_pin, dht_type), 
            one_wire_(temp_pin), daq_(), temp_(&one_wire_){}
   
  void setup_anemometer();

   float read_dht_temperature(bool S = false, bool force = false);
   float read_humidity(bool force = false);

   float read_bmp_temperature();
   float read_pressure();

   float read_temperature(uint8_t idx);
   
   void read_frame(int16_t *adcx);

   uint8_t ai_chans(int8_t i0=0, int8_t i1=-1, int8_t i2=-1, int8_t i3=-1);

   uint8_t numchans(){ return nch_; }
   int8_t* chanidx(){ return ich_; }
};

  
template <typename Comm>
class AnemometerComm
{
protected:
  Comm *comm_;  // Handles communication with computer
  Anemometer *anem_; // Handles data acquisition
  uint16_t fps_;  // Number of samples that should be read
  uint16_t avg_; // Number of samples that should be averaged
  int32_t frame_[4]; // Place to store the samples read
  char buffer_[32]; // Buffer
  
public:
  AnemometerComm(Comm *comm, Anemometer *anem):
    comm_(comm), anem_(anem){fps_=1; avg_=1;}
  
  void set_comm(Comm *comm=0){
    comm_ = comm;
  }
  
  bool available(){
    return comm_->available();
  }
  void set_avg(uint16_t avg){
    if (avg > 2000)
      avg = 1000;
    avg_ = avg;
  }

  void set_fps(uint16_t fps){
    fps_ = fps;
  }

  int parse_setvar(String cmd){
    int x;
    cmd.trim();
    comm_->println(cmd);
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
  
  void envconds(){
    comm_->println(5);
    comm_->println(anem_->read_pressure());
    comm_->println(anem_->read_humidity());
    for (int i = 0; i < 3; ++i)
      comm_->println(anem_->read_temperature(i));
    comm_->println("OK");
  }

  void scan(){
    int16_t fr[4];
    uint8_t nch = anem_->numchans();
    
    comm_->println(fps_);
    comm_->println(nch);
    
    unsigned long t1 = millis();
    for (int i = 0; i < fps_; ++i){
      for (int k = 0; k < nch; ++k){
        frame_[k] = 0;
      }
      for (int j = 0; j < avg_; ++j){
        anem_->read_frame(fr);
        for (int k = 0; k < nch; ++k){
          frame_[k] += fr[k];
        }
      }
      for (int k = 0; k < nch; ++k){
        comm_->println(frame_[k] / avg_);
      }
    }
    comm_->println( (millis() - t1) / 1000.0);
    comm_->println("OK");
  }
  
  void repl(){
    String line;
    // Let's read a lines of input
    line = comm_->readStringUntil('\n');
    line.trim();
    line.toUpperCase();
    if (line.startsWith("SET")){
      if (parse_setvar(line.substring(3))){
        comm_->println("ERR");
      }else{
        comm_->println("OK");
      }
    }else if (line.startsWith("SCAN")){
      scan();          
    }else if (line.startsWith("ENV")){
      envconds();
    }else if (line=="STATUS"){
      comm_->println("OK");
    }else{
      comm_->println("ERR");
    }
  }
       
  
};





#endif __ANEMOMETER_H__

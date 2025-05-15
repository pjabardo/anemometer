

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

  





#endif //__ANEMOMETER_H__



#include "anemometer.h"

DeviceAddress temp_probes[] ={ { 0x28, 0x8D, 0xFA, 0x79, 0x97, 0x09, 0x03, 0x9C }, 
                      { 0x28, 0xFF, 0x8E, 0xDE, 0x82, 0x15, 0x02, 0xB7 },  
                      { 0x28, 0xFF, 0xC1, 0xF8, 0x82, 0x15, 0x02, 0x48 }};

void Anemometer::setup_anemometer()
{
  // Humidity sensor
  _dht.begin();

  // BMP280
  if (!_bmp.begin(_bmp_addr)){
    // I still don't know what to do in case of failure...
  }
  _bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                   Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                   Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                   Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                   Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  

  _daq.setGain(_gain);
  
  if (!_daq.begin(_daq_addr)) {
    return;
  }

  _temp.begin();  // DS18b20 temperature sensors
  _temp.setResolution(_temp_resolution);

  _ich[0] = 0;
  _ich[1] = 1;
  _ich[2] = 2;
  _ich[3] = 3;
  _nch = 1;
  
}

void Anemometer::setup_temperature(){

  
}

float Anemometer::read_dht_temperature(bool S, bool force){
  return _dht.readTemperature();
}
float Anemometer::read_humidity(bool force){
  return _dht.readHumidity();
}

float Anemometer::read_bmp_temperature(){
  return _bmp.readTemperature(); 
}

float Anemometer::read_pressure(){
  return _bmp.readPressure();
}

float Anemometer::read_temperature(uint8_t idx){
  _temp.requestTemperaturesByAddress(temp_probes[idx]);
  return _temp.getTempC(temp_probes[idx]);
  
}
int16_t Anemometer::read_aichan(uint8_t idx){
  return _daq.readADC_SingleEnded(idx);
}

void Anemometer::read_frame(int16_t *adcx){
  for (uint8_t i  = 0; i < _nch; ++i)
    adcx[i] = _daq.readADC_SingleEnded(i);
}


uint8_t Anemometer::ai_chans(int8_t i0, int8_t i1, int8_t i2, int8_t i3)
{
  uint8_t nch = 0;
  uint8_t ich[] = {-1,-1,-1,-1};

  if (i0 >= 0 && i0 < 4)
    ich[nch++] = i0;
  
  if (i1 >= 0 && i1 < 4)
    ich[nch++] = i1;
  
  if (i2 >= 0 && i2 < 4)
    ich[nch++] = i2;
    
  if (i3 >= 0 && i3 < 4)
    ich[nch++] = i3;

  _nch = nch;
  for (int k = 0; k < 4; ++k)
    _ich[k] = ich[k];
  
  return 0;   
  
}

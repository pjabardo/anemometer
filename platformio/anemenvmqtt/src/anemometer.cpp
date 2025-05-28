

#include "anemometer.h"

DeviceAddress temp_probes[] = { { 0x28, 0x8D, 0xFA, 0x79, 0x97, 0x09, 0x03, 0x9C },
  { 0x28, 0xFF, 0x8E, 0xDE, 0x82, 0x15, 0x02, 0xB7 },
  { 0x28, 0xFF, 0xC1, 0xF8, 0x82, 0x15, 0x02, 0x48 }
};
float erra_t[] = {1, 1, 1, 1, 1, 1, 1, 1};
float errb_t[] = {0, 0, 0, 0, 0, 0, 0, 0};

void Anemometer::setup_anemometer()
{
  // Humidity sensor
  _dht.begin();

  // BMP280
  if (!_bmp.begin(_bmp_addr)) {
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


  _ich[0] = 0;
  _ich[1] = -1;
  _ich[2] = -1;
  _ich[3] = -1;
  _nch = 1;
}

void Anemometer::setup_temperature() {
  _temp.begin();  // DS18b20 temperature sensors
  _temp.setResolution(_temp_resolution);
  load_temp_sensors();

  // Identify the registered temperature sensors:
  for (uint8_t i = 0; i < MAXTCHANS; ++i){
    regtemp[i] = -1;
  }

  for (uint8_t i = 0; i < _ntaddr; ++i){
    for (uint8_t k = 0; k < NTREG; ++k){
      bool eq = true;
      for (uint8_t j = 0; j < 8; ++j){
        if (_taddr[i][j] != temp_probes[k][j]){
          eq = false;
          break;
        }
      }
      if (eq){
        regtemp[i] = k;
      }
    }
  }
}

uint8_t Anemometer::load_temp_sensors(){
  _temp.begin();
  _temp.setResolution(_temp_resolution);
  _ntaddr = _temp.getDeviceCount();
  
  _ntaddr = (_ntaddr > MAXTCHANS) ? MAXTCHANS : _ntaddr;
  
  for (uint8_t i = 0; i < _ntaddr; ++i){
    _temp.getAddress(_taddr[i], i);
  }
  
  // Set
  return _ntaddr;
}
uint8_t *Anemometer::tempaddr(uint8_t tch) {
  if (tch < _ntaddr){
    return _taddr[tch];
  }else{
    return 0;
  }
  
}
float Anemometer::read_dht_temperature(bool S, bool force) {
  return _dht.readTemperature();
}
float Anemometer::read_humidity(bool force) {
  return _dht.readHumidity();
}

float Anemometer::read_bmp_temperature() {
  return _bmp.readTemperature();
}

float Anemometer::read_pressure() {
  return _bmp.readPressure() - 368.5;
}

float Anemometer::read_temperature(uint8_t idx) {
   _temp.requestTemperaturesByAddress(_taddr[idx]);
   float T = _temp.getTempC(_taddr[idx]);
  
   int8_t idreg = regtemp[idx];
  
  if (idreg >= 0){
    // Temp sensor is registered. Correct it!
    T = erra_t[idreg]*T + errb_t[idreg];
  }
  return T;

}
int16_t Anemometer::read_aichan(uint8_t idx) {
  return _daq.readADC_SingleEnded(idx);
}

void Anemometer::read_frame(int16_t *adcx) {
  for (uint8_t i  = 0; i < _nch; ++i)
    adcx[i] = _daq.readADC_SingleEnded(_ich[i]);
}

int Anemometer::chanidx(uint8_t i) {
  if (i < 0 || i >= _nch) {
    return -201;
  } else {
    return _ich[i];
  }
}
void Anemometer::clear_aichans() {
  _nch = 0;
  for (char i = 0; i < 4; ++i) {
    _ich[i] = 0;
  }
}

int Anemometer::add_aichan(uint8_t idx) {
  if (_nch > 3) {
    return -202;
  }
  for (char i = 0; i < _nch; ++i) {
    if (_ich[i] == idx) {
      return -203;
    }
  }
  _ich[_nch++] = idx;
  return 0;
}

void Anemometer::set_temp_res(uint8_t tres){
  if (tres < 9){
    tres = 9;
  }else if (tres > 12){
    tres = 12;
  }
  
  _temp.setResolution(tres);
  _temp_resolution = tres;
}

int8_t Anemometer::ai_chans(int8_t i0, int8_t i1, int8_t i2, int8_t i3)
{
  int8_t nch = 0;
  int8_t ich[] = { -1, -1, -1, -1};

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


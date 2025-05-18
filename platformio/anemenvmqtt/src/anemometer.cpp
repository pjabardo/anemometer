

#include<WiFi.h>

#include "anemometer.h"

DeviceAddress temp_probes[] = { { 0x28, 0x8D, 0xFA, 0x79, 0x97, 0x09, 0x03, 0x9C },
  { 0x28, 0xFF, 0x8E, 0xDE, 0x82, 0x15, 0x02, 0xB7 },
  { 0x28, 0xFF, 0xC1, 0xF8, 0x82, 0x15, 0x02, 0x48 }
};

Anemometer::Anemometer(uint8_t bmp_addr, uint8_t dht_pin, uint8_t dht_type, 
            uint8_t temp_pin, uint8_t daq_addr, adsGain_t gain,
            uint8_t temp_resolution):
            _bmp_addr(bmp_addr), _dht_pin(dht_pin), _dht_type(dht_type), 
            _temp_pin(temp_pin), _daq_addr(daq_addr), _gain(gain), 
            _temp_resolution(temp_resolution), _bmp(), _dht(dht_pin, dht_type), 
            _one_wire(temp_pin), _daq(), _temp(&_one_wire)
{
  setup_anemometer();
  setup_temperature();

}
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
}

uint8_t Anemometer::load_temp_sensors(){
  _temp.begin();
  _temp.setResolution(_temp_resolution);
  _ntaddr = _temp.getDeviceCount();
  
  _ntaddr = (_ntaddr > MAXTCHANS) ? MAXTCHANS : _ntaddr;
  
  for (uint8_t i = 0; i < _ntaddr; ++i){
    _temp.getAddress(_taddr[i], i);
  }
  
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
  return _bmp.readPressure();
}

float Anemometer::read_temperature(uint8_t idx) {
  _temp.requestTemperaturesByAddress(_taddr[idx]);
  return _temp.getTempC(_taddr[idx]);

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




MQTTAnem::MQTTAnem(const char *bname, Anemometer *anem, PubSubClient *client):
      _anem(anem), _client(client), _bname(bname)
{
  // Let's save the retained variables:
  char *topic = _buf;
  char *buf   = _buf2;
}



void MQTTAnem::initialize(Anemometer *anem, PubSubClient *client){
  _client = client;
  _anem = _anem;
}

void MQTTAnem::publish_params()
{
  char *topic = _buf;
  char *buf = _buf2;

  _client->loop();

  snprintf(topic, 31, "%s/P/S", _bname);
  _client->publish(topic, "PT", true);
  Serial.print(topic);
  Serial.println("  PT");

  snprintf(topic, 31, "%s/H/S", _bname);
  _client->publish(topic, "HT", true);
  Serial.print(topic);
  Serial.println("  HT");

  // Temperature sensors
  snprintf(topic, 31, "%s/T/N", _bname);
  snprintf(buf, 31, "%d", _anem->numtemp());
  Serial.print(topic);
  Serial.print("  N = ");
  Serial.println(buf);
  _client->publish(topic, buf, true);

  // Write the Addresses of every DS18B20 temperature sensor
  for (int i = 0; i < _anem->numtemp(); ++i){
    snprintf(topic, 31, "%s/T/ID%d", _bname, i);
    _client->publish(topic, _anem->tempaddr(i), 8, true);
    Serial.print(topic);
    Serial.print("  ");
    Serial.println(i);
  }

  // Analog channels:
  // Implement it later on...

}

void MQTTAnem::clear_buffer(char *b, int n)
{
  for (int i = 0; i < n; ++i)
    b[i] = 0;
}


void MQTTAnem::loop()
{
  float x;
  char *topic = _buf;
  char *buf = _buf2;

  _client->loop();

  // Read pressure:
  x = _anem->read_pressure();
  Serial.println(x);
  dtostrf(x, 10, 2, buf);
  snprintf(topic, 31, "%s/P/P", _bname);
  _client->publish("topic", buf);

  x = _anem->read_bmp_temperature();
  Serial.println(x);
  dtostrf(x, 10, 3, buf);
  snprintf(topic, 31, "%s/P/T", _bname);
  _client->publish("topic", buf);

  x = _anem->read_humidity();
  Serial.println(x);
  dtostrf(x, 10, 3, buf);
  snprintf(topic, 31, "%s/H/H", _bname);
  _client->publish("topic", buf);

  x = _anem->read_dht_temperature();
  Serial.println(x);
  dtostrf(x, 10, 3, buf);
  snprintf(topic, 31, "%s/H/T", _bname);
  _client->publish("topic", buf);

  for (int i = 0; i < _anem->numtemp(); ++i){
    x = _anem->read_temperature(0);
    Serial.println(x);
    dtostrf(x, 10, 3, buf);
    snprintf(topic, 31, "%s/T/T%d", _bname, i);
    _client->publish("topic", buf);
  }

}
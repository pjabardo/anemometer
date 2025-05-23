
#include "mqtt_anem.h"


void serial_print_topic(const char *topic, const char *buf){
  Serial.print("Topic: ");
  Serial.print(topic);
  Serial.print(" -> ");
  Serial.println(buf);
}

void serial_print_topic(const char *topic, const char *buf, int len){
  Serial.print("Topic: ");
  Serial.print(topic);
  Serial.print(" -> ");
  for (int i = 0; i < len; ++i){
    Serial.print(buf[i]);
  }
  Serial.println();
}

MQTTAnem::MQTTAnem(const char *bname, Anemometer *anem, PubSubClient *client):
      _anem(anem), _client(client), _bname(bname), _avg(200)
{ 
  // BMP channels
  _p = _chans; // BMP P, T
  _h = _p + 2; //DHT H, T
  _a = _h + 2; // ADS115 A0, A1, A2, A3, A4;
  _t = _a + 4; // DS18B20 T0 - T7  (if present)
 
  // Set the callback:
  set_callback();

}

void MQTTAnem::bmp_chans(char *s){
  uint8_t i = 0;
  _p[0] = 0;
  _p[1] = 0;
  for (char *c = s; *c != 0; ++c){
    if (i == 2){
      break;
    }
    if (*c=='1'){
      _p[i] = 1;
    }
    ++i;
  }
}

void MQTTAnem::dht_chans(char *s){
  uint8_t i = 0;
  _h[0] = 0;
  _h[1] = 0;
  for (char *c = s; *c != 0; ++c){
    if (i == 2){
      break;
    }
    if (*c=='1'){
      _h[i] = 1;
    }
    ++i;
  }

}

void MQTTAnem::ai_chans(char *s){
  uint8_t i = 0;
  for (i = 0; i < 4; ++i){
    _a[i] = 0;
  }

  _anem->clear_aichans();
    
  i = 0;
  for (char *c = s; *c != 0; ++c){
    if (i == 4){
      break;
    }
    if (*c=='1'){
      _anem->add_aichan(i);
      _a[i] = 1;
    }
    ++i;
  }

}

void MQTTAnem::temp_chans(char *s){
  uint8_t ntemp = _anem->numtemp();
  uint8_t i = 0;

  for (i = 0; i < 8; ++i){
    _t[i] = 0;
  }
  
  i = 0;
  for (char *c = s; *c != 0; ++c){
    if (i == ntemp){
      break;
    }
    if (*c == '1'){
      _t[i] = 1;
    }
    ++i;
  }

}

void MQTTAnem::fill_chans(char *buf, uint8_t *c, uint8_t n){
  for (uint8_t i = 0; i < n; ++i){
    if (c[i] != 0){
      buf[i] = '1';
    } else {
      buf[i] = '0';
    }
  }
}

void MQTTAnem::set_avg(uint16_t avg){
  if (avg <= 0){
    _avg = 1;
  } else if (avg > 2000){
    _avg = 2000;
  }else{
    _avg = avg;
  }

}

void MQTTAnem::scan(){
  int16_t fr[4];
  uint8_t nch = _anem->numchans();
  for (uint8_t i = 0; i < nch; ++i){
    _frame[i] = 0;
  }
  for (uint16_t k = 0; k < _avg; ++k){
    _anem->read_frame(fr);
    for (uint8_t i = 0; i < nch; ++i){
      _frame[i] += fr[i];
    }
  }
  for (uint8_t i = 0; i < nch; ++i){
    _frame[i] = _frame[i] / _avg;
  }
}


void MQTTAnem::_callback(char* topic, uint8_t *payload, unsigned int length){
  Serial.println("CALLBACK!!!");
  serial_print_topic(topic, (char *) payload, length);
  Serial.println(length);
  Serial.println("END CALLBACK!!!");
  int n = strlen(_bname);
  char c = topic[n+1];
  if (c == 'P'){
    _cb_bmp(topic+n+3, payload, length);
  } else if (c == 'H') {
    _cb_dht(topic+n+3, payload, length);
  } else if (c == 'T') {
    _cb_temp(topic+n+3, payload, length);
  } else if (c == 'A') {
    _cb_ai(topic+n+4, payload, length);
  } else{
    // Shouldn't be here! I don't know what to do.
    // For now I won't do anything.
  }
}

void MQTTAnem::set_callback(){
  _client->setCallback([this](char *topic, uint8_t *payload, unsigned int length) { 
    _callback(topic, payload, length);
  });
}

void MQTTAnem::_cb_bmp(char* topic, uint8_t *payload, unsigned int length){
  _p[0] = 0;
  _p[1] = 0;
  for (uint8_t i = 0; i < length; ++i){
    if (payload[i] == '1'){
      _p[i] = 1;
    }
    
    
  }
}

void MQTTAnem::_cb_dht(char* topic, uint8_t *payload, unsigned int length){
  _h[0] = 0;
  _h[1] = 0;
  for (uint8_t i = 0; i < length; ++i){
    if (payload[i] == '1'){
      _h[i] = 1;
    }
  }
}

void MQTTAnem::_cb_ai(char* topic, uint8_t *payload, unsigned int length){
  uint8_t i;
  int32_t num;
  for (i = 0; i < length; ++i){
    _buf[i] = payload[i];
  }
  _buf[length] = 0;

  if (topic[0]== 'A' && topic[1] == 'V' && topic[2] == 'G'){
        num = atoi(_buf);
    set_avg(num);
  } else if (topic[0] == 'C' && topic[1] == 'H'){
    ai_chans(_buf);
  }

  
}

void MQTTAnem::_cb_temp(char* topic, uint8_t *payload, unsigned int length){
  uint8_t i;
  int32_t num;
  for (i = 0; i < length; ++i){
    _buf[i] = payload[i];
  }
  _buf[length] = 0;

  if (topic[0] == 'C' && topic[1] == 'H'){
    serial_print_topic(topic, _buf);
    temp_chans(_buf);
  }

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
  // Read only the pressure. 
  // The temperature is not very good: the board is heated...
  snprintf(topic, 31, "%s/P/CHANS", _bname);
  _client->publish(topic, "10", true);
  bmp_chans((char *) "10");
  serial_print_topic(topic, "10");
  _client->subscribe(topic);

  // Store UNITS
  snprintf(topic, 31, "%s/P/UNIT", _bname);
  _client->publish(topic, "Pa", true);
  serial_print_topic(topic, "P -> Pa");

  snprintf(topic, 31, "%s/P/TUNIT", _bname);
  _client->publish(topic, "oC", true);
  serial_print_topic(topic, "T -> oC");

  _client->loop();

  snprintf(topic, 31, "%s/H/CHANS", _bname);
  _client->publish(topic, "11", true);
  dht_chans((char *) "11");
  serial_print_topic(topic, "11");
  _client->subscribe(topic);

  snprintf(topic, 31, "%s/H/UNIT", _bname);
  _client->publish(topic, "%", true);
  serial_print_topic(topic, "H -> %");

  snprintf(topic, 31, "%s/H/TUNIT", _bname);
  _client->publish(topic, "oC", true);
  serial_print_topic(topic, "T -> oC");


  _client->loop();

  // We will not read the analog inputs.
  snprintf(topic, 31, "%s/AI/N", _bname);
  snprintf(buf, 31, "%d", _anem->numchans());
  _client->publish(topic, buf, true);
  serial_print_topic(topic, buf);


  snprintf(topic, 31, "%s/AI/CHANS", _bname);
  clear_buffer(buf, 5);
  for (uint8_t i = 0; i < 4; ++i){
    buf[i] = '0';
  }
  buf[0] = '0';
  buf[4] = 0;
  ai_chans(buf);
  _client->publish(topic, buf, true);
  serial_print_topic(topic, buf);
  _client->subscribe(topic);
  _client->loop();


  snprintf(topic, 31, "%s/AI/AVG", _bname);
  snprintf(buf, 31, "%d", _avg);
  _client->publish(topic, buf, true);
  serial_print_topic(topic, buf);
  _client->subscribe(topic);
  _client->loop();

  snprintf(topic, 31, "%s/AI/GAIN", _bname);
  snprintf(buf, 31, "2");
  _client->publish(topic, buf, true);

  snprintf(topic, 31, "%s/AI/RANGE", _bname);
  snprintf(buf, 31, "2.048");
  _client->publish(topic, buf, true);

  snprintf(topic, 31, "%s/AI/COUNTS", _bname);
  snprintf(buf, 31, "32768");
  _client->publish(topic, buf, true);

  snprintf(topic, 31, "%s/AI/UNIT", _bname);
  snprintf(buf, 31, "V");
  _client->publish(topic, buf, true);

  
  // Temperature sensors
  uint8_t nt = _anem->numtemp();
  snprintf(topic, 31, "%s/T/N", _bname);
  snprintf(buf, 31, "%d", nt);
  _client->publish(topic, buf, true);
  for (uint8_t i = 0; i < nt; ++i){
    buf[i] = '1';
  }
  buf[nt] = 0;

  snprintf(topic, 31, "%s/T/CHANS", _bname);
  clear_buffer(buf, 32);
  for (uint8_t i = 0; i < nt; ++i){
    buf[i] = '1';
  }
  temp_chans(buf);
  _client->publish(topic, buf, true);
  serial_print_topic(topic, buf);
  _client->subscribe(topic);
  _client->loop();

  // Write the Addresses of every DS18B20 temperature sensor
  for (uint8_t i = 0; i < _anem->numtemp(); ++i){
    snprintf(topic, 31, "%s/T/ID%d", _bname, i);
    clear_buffer(buf, 32);      
    snprintf(buf, 31, "%d", i);
    _client->publish(topic, _anem->tempaddr(i), 8, true);
    //serial_print_topic(topic, buf);

  }

  snprintf(topic, 31, "%s/T/UNIT", _bname);
  _client->publish(topic, "oC", true);
  serial_print_topic(topic, "T -> oC");

  _client->loop();

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
  if (_p[0]){
    x = _anem->read_pressure();
    dtostrf(x, 10, 2, buf);
    snprintf(topic, 31, "%s/P/P", _bname);
    _client->publish(topic, buf);
    serial_print_topic(topic, buf);
  }

  if (_p[1]){
    x = _anem->read_bmp_temperature();
    dtostrf(x, 10, 3, buf);
    snprintf(topic, 31, "%s/P/T", _bname);
    _client->publish(topic, buf);
    serial_print_topic(topic, buf);
  } 

  if (_h[0]){
    x = _anem->read_humidity();
    dtostrf(x, 10, 3, buf);
    snprintf(topic, 31, "%s/H/H", _bname);
    _client->publish(topic, buf);
    serial_print_topic(topic, buf);
  }

  if (_h[1]){
    x = _anem->read_dht_temperature();
    dtostrf(x, 10, 3, buf);
    snprintf(topic, 31, "%s/H/T", _bname);
    _client->publish(topic, buf);
    serial_print_topic(topic, buf);
  }

  uint8_t nch = _anem->numchans();
  if (nch > 0){
    scan();
    int32_t *fr = frames();
    for (uint8_t  i = 0; i < nch; ++i){
      uint8_t idx = _anem->chanidx(i);
      snprintf(topic, 31, "%s/AI/AI%d", _bname, idx);
      x = _anem->ai_volts(fr[i]); // Calculate the voltage
      dtostrf(x, 12, 6, buf);
      _client->publish(topic, buf);
      serial_print_topic(topic, buf);
    }
  }

  nch = _anem->numtemp();
  for (int i = 0; i < nch; ++i){
    if (_t[i]){
      x = _anem->read_temperature(0);
      dtostrf(x, 10, 3, buf);
      snprintf(topic, 31, "%s/T/T%d", _bname, i);
      _client->publish(topic, buf);
      serial_print_topic(topic, buf);
    }
  }

}

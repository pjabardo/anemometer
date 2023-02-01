

#ifndef __ANEMOMETER_H__
#define __ANEMOMETER_H__

#include <cstdint>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_ADS1X15.h>

template<TComm>
class Anemometer
{
  TComm *comm;
  DHT *dht;
  Adafruit_BMP280 *bmp;
  OneWire *onew;
  DallasTemperature *temp;
  Adafruit_ADS1115 *ads;

  int ntemps;
  DeviceAddress probes[10];
  
 public:
  Anemometer(TComm *comm, DHT *dht, Adafruit_BMP280 *bmp,
	     DallasTemperature *temp, Adafruit_ADS1115 *ads):
  comm(comm), dht(dht), bmp(bmp), onew(onew), temp(temp), ads(ads){}

  int scan_temp_address(int nmax=10);
  
};

#endif // __ANEMOMETER_H__



#ifndef __ANEMOMETER_H__
#define __ANEMOMETER_H__

#include <DallasTemperature.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_ADS1X15.h>
#include <DHT.h>

class Anemometer
{
protected:
  Stream *comm_;
  Adafruit_BMP280 *press_;
  Adafruit_ADS1115 *daq_;
  DallasTemperature *temp_;
  DHT *hum_;

public:
  Anemometer(Stream *comm, Adafruit_BMP280 *press=0, Adafruit_ADS1115 *daq=0,
	     DallasTemperature *temp=0, DHT *hum=0):
    comm_(comm), press_(press), daq_(daq), temp_(temp), hum_(hum){}

  
};

#endif __ANEMOMETER_H__

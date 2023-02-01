
// Implemens class that interfaces with anemometer

#include "anemometer.h"

int Anemometer::scan_temp_address(){
  DeviceAddress addr;
  int nmax  = 10;
  int nd = temp->getDeviceCount();

  nd = (nd < nmax) ? nd : nmax;

  int nfound = 0;
  addr = probes[nfound];
  
  while(onew.search(addr)){
    if (OneWire::crc8(addr, 7) != addr[7]) {
      continue;
    }else{
      nfound++;
      addr = probes[nfound];
    }
  }

  return nfound;
}

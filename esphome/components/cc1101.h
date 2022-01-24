#ifndef CC1101TRANSCIEVER_H
#define CC1101TRANSCIEVER_H

#include <ELECHOUSE_CC1101_SRC_DRV.h>

int CC1101Transceiver_module_number = 0;
class CC1101Transceiver : public Component {
  int _SCK;
  int _MISO;
  int _MOSI;
  int _CSN;
  int _GDO0;
  int _GDO2;
  float _moduleNumber;
  void setup(){
    pinMode(_GDO0, OUTPUT);
    pinMode(_GDO2, INPUT);
    ELECHOUSE_cc1101.addSpiPin(_SCK, _MISO, _MOSI, _CSN, _moduleNumber);
    ELECHOUSE_cc1101.addGDO(_GDO0, _GDO2, _moduleNumber);
    select();
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.SetRx(); 
  }
public:
  CC1101Transceiver(int SCK, int MISO, int MOSI, int CSN, int GDO0, int GDO2) {
    _SCK = SCK;
    _MISO = MISO;
    _MOSI = MOSI;
    _CSN = CSN;
    _GDO0 = GDO0;
    _GDO2 = GDO2;
    _moduleNumber = CC1101Transceiver_module_number++;
  }
  void select() {
    ELECHOUSE_cc1101.setModul(_moduleNumber);
  }
  void setMHZ(float freq) {
    select();
    ELECHOUSE_cc1101.setMHZ(freq);
  }
  void beginTransmission() {
    select();
    ELECHOUSE_cc1101.SetTx();
  }
  void endTransmission() {
    select();
    ELECHOUSE_cc1101.SetRx();
    ELECHOUSE_cc1101.SetRx(); // yes, twice
  }  
};

#endif

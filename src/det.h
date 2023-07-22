#ifndef Gobbi_H
#define Gobbi_H
#include <iostream>
#include <string>
#include "HINP.h"
#include "histo.h"
#include "silicon.h"
#include "correl2.h"
#include "calibrate.h"

//Nicolas Dronchi 2022_04_04
//Class written to handle all specifics of the Gobbi array
//such as communicating with HINP, calibrations, Checking for charge sharing in neighbor
//calculating geometry

class Gobbi
{
 public:
  double Targetdist;
  float TargetThickness;

  Gobbi(histo * Histo1);
  ~Gobbi();

  bool unpack(unsigned short *point);
  histo * Histo;


private:

  HINP * SiADC;
  caen * CsIADC;
  TDC1190 * TDC;

  correl2 Correl;

  void corr_14O();
};


#endif

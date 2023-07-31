#ifndef det_H
#define det_H
#include <iostream>
#include <string>
#include "HINP.h"
#include "TDC1190.h"
#include "caen.h"
#include "gobbi.h"
#include "histo.h"
#include "correl2.h"

//Nicolas Dronchi 2022_04_04
//such as communicating with HINP, calibrations, Checking for charge sharing in neighbor
//calculating geometry

class det
{
 public:
  double Targetdist;
  float TargetThickness;

  det(histo * Histo1);
  ~det();

  bool unpack(unsigned short *point);
  histo * Histo;
  gobbi * Gobbi;

private:
  HINP * SiADC;
  caen * CsIADC;
  TDC1190 * TDC;
  correl2 Correl;

  void corr_14O();
  void corr_15O();
  void corr_15F();
};


#endif

#ifndef Gobbi_H
#define Gobbi_H
#include <iostream>
#include <string>
#include "histo.h"
#include "silicon.h"
#include "calibrate.h"

class Gobbi
{
 public:
  double Targetdist;
  float TargetThickness;

  Gobbi(histo * Histo1);
  ~Gobbi();
  histo * Histo;
  silicon * Silicon[4];

  calibrate * FrontEcal;
  calibrate * BackEcal;
  calibrate * DeltaEcal;
  calibrate * FrontTimecal;
  calibrate * BackTimecal;
  calibrate * DeltaTimecal;


  void SetTarget(double Targetdist, float TargetThickness);

  void addFrontEvent(int quad, unsigned short chan, unsigned short high, unsigned short low, unsigned short time);
  void addBackEvent(int quad, unsigned short chan, unsigned short high, unsigned short low, unsigned short time);
  void addDeltaEvent(int quad, unsigned short chan, unsigned short high, unsigned short low, unsigned short time);
  void addCsIEvent(int quad, unsigned short chan, unsigned short high, unsigned short low, unsigned short time);

  void reset();
  void SiNeigbours();
  int matchTele();

};


#endif

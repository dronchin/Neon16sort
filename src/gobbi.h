#ifndef Gobbi_H
#define Gobbi_H
#include <iostream>
#include <string>
#include "histo.h"
#include "silicon.h"
#include "calibrate.h"

//two structures for matching up CsI times and Energies
struct dataE
{
  int ienergy;
  int id;
  int itime;
  float energy;
  float time;
};
struct dataT
{
  int itime;
  int id;
};


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
  calibrate * CsIEcal;

  calibrate * FrontTimecal;
  calibrate * BackTimecal;
  calibrate * DeltaTimecal;
  calibrate * CsITimecal;

  void SetTarget(double Targetdist, float TargetThickness);
  void reset();

  void addFrontEvent(int quad, unsigned short chan, unsigned short high, unsigned short low, unsigned short time);
  void addBackEvent(int quad, unsigned short chan, unsigned short high, unsigned short low, unsigned short time);
  void addDeltaEvent(int quad, unsigned short chan, unsigned short high, unsigned short low, unsigned short time);
  void addCsIEvent(int quad, unsigned short chan, unsigned short high, unsigned short low, unsigned short time);


  void SiNeigbours();
  int matchTele();
  int multiSiSi;
  int multiSiSiCsI;

  int NsimpleSiSiCsI = 0;
  int NmultiSiSiCsI = 0;
  int NsimpleSiSi = 0;
  int NmultiSiSi = 0;

  int NE;
  int NT;
  dataE DataE[56];
  dataT DataT[56];
  void MatchCsIEnergyTime();

};


#endif

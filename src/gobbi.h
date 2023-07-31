#ifndef Gobbi_H
#define Gobbi_H
#include <iostream>
#include <string>
#include "histo.h"
#include "telescope.h"
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

class gobbi
{
 public:
  float TargetThickness;

  gobbi(histo * Histo1);
  ~gobbi();
  histo * Histo;
  telescope * Telescope[4];

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

  int analyze();
  void SiNeighbours();
  int matchTele();

  int multidEE;
  int multiECsI;
  int NsimpleECsI = 0;
  int NmultiECsI = 0;
  int NsimpledEE = 0;
  int NmultidEE = 0;

  int NE;
  int NT;
  dataE DataE[56];
  dataT DataT[56];
  void MatchCsIEnergyTime();

};


#endif

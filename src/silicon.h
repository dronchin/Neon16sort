#ifndef _silicon
#define _silicon

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include "TMath.h"
#include <cmath>
#include "TRandom.h"
#include "elist.h"
#include "solution.h"
#include "pid.h"
#include "losses.h"
using namespace std;

/*
//structure for storing zlines
struct lines
{
  int n; //number of points
  float *x; //pointer to x array
  float *y; //pointer to y array
};
*/

class silicon
{
 public:

  float TargetThickness;

  silicon(histo * Histo1);
  ~silicon();
  histo * Histo;
  pid * Pid;
  int id;
  CLosses * losses;
  calibrate * FrontEcal;
  calibrate * BackEcal;
  calibrate * DeltaEcal;
  calibrate * FrontTimecal;
  calibrate * BackTimecal;
  calibrate * DeltaTimecal;

  void SetTarget(double Targetdist, float thick);
  void reset();
  void SiNeigbours();



//I stopped here 7/17
//need to add method to match events, steal these from gobbi.cpp
  void Reduce();
  int simpleFront();
  int multiHit();

  int getPID();
  int calcEloss();





  float maxFront;
  float maxBack;
  float maxDelta;
  int imaxFront;
  int imaxBack;
  int imaxDelta;
  int multFront;
  int multBack;
  int multDelta;

  elist Front;
  elist Back;
  elist Delta;

  solution Solution[10];
  int Nsolution;



  int simpleFrontBack();
  void position(int);
  void positionC(int);

 private:
  int FrontLow[4];
  int FrontHigh[4];
  int BackLow[4];
  int BackHigh[4];

  //position
  float Xcenter; // center of detector in cm along x axis
  float Ycenter; // center of detector in cm along y axis
  float SiWidth;
  TRandom *Ran;

  //for nested loops
  int NestDim;
  void loop(int);
  int NestArray[50];
  int arrayD[50];
  int arrayB[50];
  float deMin;
  int dstripMin;

};
#endif

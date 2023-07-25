#ifndef _telescope
#define _telescope

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

class telescope
{
 public:
  telescope(float);
  ~telescope();
  void reset();
  void init(int);
  void Reduce();
  int simpleFront();
  int multiHit();
  void SetTargetDistance(double);
  int getPID();
  int calcEloss();

  CLosses * losses;
  float TargetThickness;

  int id;
  float maxFront;
  float maxBack;
  float maxDelta;
  int imaxFront;
  int imaxBack;
  int imaxDelta;

  int multFront;
  int multBack;
  int multDelta;
  int multCsI;

  elist Front;
  elist Back;
  elist Delta;
  elist CsI;

  solution Solution[10];
  int Nsolution;

  pid * Pid;

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
  int arrayD[50]; //matches dE to E
  int arrayB[50]; //matches Front to Back
  int arrayC[50]; //matches CsI to front/back Pair
  float deMin;
  int dstripMin;

};
#endif

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
#include "calibrate.h"

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
  telescope();
  ~telescope();
  void init(int);
  void SetTarget(double, float);
  void reset();
  void Reduce();

  int testingHitE();
  int simpledEE();
  int simpleECsI();
  int multiHitdEE(); //+loop is private
  int multiHitECsI(); //+loop is private
  void load(int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int);

  int getPID();
  int calcEloss();

  CLosses * Targlosses;
  CLosses * PbSnlosses;
  CLosses * Silosses;
  float TargetThickness;

  calibrate * calCsi_d;
  calibrate * calCsi_t;
  calibrate * calCsi_Alpha;
  float light2energy(int,int,int,float);

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
  int NSisolution;
  int Nsolution;

  pid * Pid;
  pid * PidECsI[4];

  int simpleFrontBack();
  void position(int);

 private:
  calibrate* calCsI_Alpha;
  calibrate* calCsI_d;
  calibrate* calCsI_t;
  calibrate* calCsI_3He;


  int FrontLow[4];
  int FrontHigh[4];
  int BackLow[4];
  int BackHigh[4];

  //position
  float Xcenter; // center of detector in cm along x axis
  float Ycenter; // center of detector in cm along y axis
  float SiWidth;
  float WWidth;
  TRandom *Ran;

  //for nested loops
  int NestDim;
  void loop(int);
  int NestArray[50];
  int arrayD[50]; //matches dE to E
  int arrayB[50]; //matches Front to Back
  float deMin;
  int dstripMin;
};
#endif

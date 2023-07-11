#ifndef histo_
#define histo_
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include "TH1F.h"
#include "TF1.h"
#include "TH1I.h"
#include "TH2I.h"
#include "TFile.h"
#include "TGraph.h"

using namespace std;

class histo
{
protected:

  TFile * file_read; //!< output root file

public:
  histo();  //!< constructor
  ~histo();

  //manually enter the number of chips and number of channels
  static const int E_boardnum = 8;
  static const int dE_boardnum = 4;
  static const int boardnum = 16;
  static const int channum = 32; //number of channels on each board

  TDirectoryFile * dirSummary;

  TDirectory * dir1dFrontE_R;
  TDirectory * dir1dBackE_R;
  TDirectory * dir1dDeltaE_R;

  TDirectory * dir1dFrontlowE_R;
  TDirectory * dir1dBacklowE_R;
  TDirectory * dir1dDeltalowE_R;

  TDirectory * dir1dFrontE_cal;
  TDirectory * dir1dBackE_cal;
  TDirectory * dir1dDeltaE_cal;

  TDirectory * dir1dFrontTime_R;
  TDirectory * dir1dBackTime_R;
  TDirectory * dir1dDeltaTime_R;


  TDirectoryFile * dirDEEplots; //!< directory for deltaE-E plots used in particle identificaiton
  TDirectoryFile * dirhitmaps; //!< directory for all particle type hitmaps

  TDirectoryFile * dirInvMass; //!< directory for all correlations and inv-mass
  TDirectory * dir14O

  TH2I * sumFrontE_R;
  TH2I * sumBackE_R;
  TH2I * sumDeltaE_R;

  TH2I * sumFrontE_cal;
  TH2I * sumBackE_cal;
  TH2I * sumDeltaE_cal;

  TH2I * sumFrontTime_R;
  TH2I * sumFrontTime_cal;
  TH2I * sumBackTime_R;
  TH2I * sumBackTime_cal;
  TH2I * sumDeltaTime_R;
  TH2I * sumDeltaTime_cal;

  //Energies  (quadrant, channum)
  TH1I * FrontE_R[4][channum];
  TH1I * FrontElow_R[4][channum];
  TH1I * FrontTime_R[4][channum];
  TH1I * FrontE_cal[4][channum];

  TH1I * BackE_R[4][channum];
  TH1I * BackElow_R[4][channum];
  TH1I * BackTime_R[4][channum];
  TH1I * BackE_cal[4][channum];

  TH1I * DeltaE_R[4][channum];
  TH1I * DeltaElow_R[4][channum];
  TH1I * DeltaTime_R[4][channum];
  TH1I * DeltaE_cal[4][channum];

  //DeltaE-E plots
  TH2I * DEE[4];
  TH1I * timediff[4];

  TH2I * xyhitmap;
  TH2I * protonhitmap;

  //correlations and inv-mass
  //O14
  TH1I * Erel_14O_p13N;
  TH1I * Ex_14O_p13N;
  TH1I * ThetaCM_14O_p13N;
  TH1I * VCM_14O_p13N;
  TH2I * Erel_p13N_costhetaH;

};
#endif

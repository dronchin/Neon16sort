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
  static const int NCsI = 16; //Num of CsI channels 

  //all for Gobbi array
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

  TDirectory * dir1dCsI_Energy;
  TDirectory * dir1dCsI_Time;


  TDirectoryFile * dirDEEplots; //!< directory for deltaE-E plots used in particle identificaiton
  TDirectoryFile * dirhitmaps; //!< directory for all particle type hitmaps

  TDirectoryFile * dirInvMass; //!< directory for all correlations and inv-mass
  TDirectory * dir14O;
  TDirectory * dir15F;

//  ^ directories
//_|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|
//___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|__
//_|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|
//  v summaries 

  //all Gobbi summaries
  //Energies, Raw+Calibrated
  TH2I * sumFrontE_R;
  TH2I * sumBackE_R;
  TH2I * sumDeltaE_R;

  TH2I * sumFrontE_cal;
  TH2I * sumBackE_cal;
  TH2I * sumDeltaE_cal;

  TH2I * sumCsIE_R;
  TH2I * sumCsIE_cal;

  //Gobbi times
  TH2I * sumFrontTime_R;
  TH2I * sumFrontTime_cal;
  TH2I * sumBackTime_R;
  TH2I * sumBackTime_cal;
  TH2I * sumDeltaTime_R;
  TH2I * sumDeltaTime_cal;
  TH2I * sumCsITime_R;
  TH2I * sumCsITime_cal;

//  ^ summaries
//_|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|
//___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|__
//_|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|
//  v 1d spectra


  //create all Gobbi 1d spectra
  //Energies  (quadrant, channum)
  TH1I * FrontE_R[5][channum];
  TH1I * FrontElow_R[5][channum];
  TH1I * FrontTime_R[5][channum];
  TH1I * FrontE_cal[5][channum];

  TH1I * BackE_R[5][channum];
  TH1I * BackElow_R[5][channum];
  TH1I * BackTime_R[5][channum];
  TH1I * BackE_cal[5][channum];

  TH1I * DeltaE_R[5][channum];
  TH1I * DeltaElow_R[5][channum];
  TH1I * DeltaTime_R[5][channum];
  TH1I * DeltaE_cal[5][channum];
  //create all the CsI 1d spectra
  TH1I * CsI_Energy_R_um[NCsI];
  TH1I * CsI_Energy_R[NCsI];
  TH1I * CsI_Energy_cal[NCsI];
  TH1I * CsI_Time_R_um[NCsI];
  TH1I * CsI_Time_R[NCsI];
  TH1I * CsI_Time_cal[NCsI];



  //DeltaE-E plots
  TH2I * DEE[5];
  TH1I * timediff[5];
  TH2I * DEE_CsI[4][4];
  TH1I * timediff_CsI[4][4];

  TH2I * testinghitmap;
  TH2I * xyhitmap;
  TH2I * protonhitmap;
  TH2I * oxygenhitmap;

  //correlations and inv-mass
  //O14
  TH1I * Erel_14O_p13N;
  TH1I * Ex_14O_p13N;
  TH1I * ThetaCM_14O_p13N;
  TH1I * VCM_14O_p13N;
  TH2I * Erel_p13N_costhetaH;

  //F15
  TH1I * Erel_15F_p14O;
  TH1I * Ex_15F_p14O;
  TH1I * ThetaCM_15F_p14O;
  TH1I * VCM_15F_p14O;
  TH2I * Erel_p14O_costhetaH;

};
#endif

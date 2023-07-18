#include "histo.h"

histo::histo()
{
  //create root file
  file_read = new TFile ("sort.root","RECREATE");
  file_read->cd();

  //create subdirectories to store arrays of spectra
  dirSummary = new TDirectoryFile("GobbiSum", "GobbiSum"); //name, title
  //make subdirectories
  //Energies and DeltaEnergies, Raw+Calibrated
  dir1dFrontE_R = dirSummary->mkdir("1dFrontE_R","1dFrontE_R");
  dir1dBackE_R = dirSummary->mkdir("1dBackE_R","1dBackE_R");
  dir1dDeltaE_R = dirSummary->mkdir("1dDeltaE_R","1dDeltaE_R");

  dir1dFrontlowE_R = dirSummary->mkdir("1dFrontlowE_R","1dFrontlowE_R");
  dir1dBacklowE_R = dirSummary->mkdir("1dBacklowE_R","1dBacklowE_R");
  dir1dDeltalowE_R = dirSummary->mkdir("1dDeltalowE_R","1dDeltalowE_R");

  dir1dFrontE_cal = dirSummary->mkdir("1dFrontE_cal","1dFrontE_cal");
  dir1dBackE_cal = dirSummary->mkdir("1dBackE_cal","1dBackE_cal");
  dir1dDeltaE_cal = dirSummary->mkdir("1dDeltaE_cal","1dDeltaE_cal");

  //raw time summaries
  dir1dFrontTime_R = dirSummary->mkdir("1dFrontTime_R","1dFrontTime_R");
  dir1dBackTime_R = dirSummary->mkdir("1dBackTime_R","1dBackTime_R");
  dir1dDeltaTime_R = dirSummary->mkdir("1dDeltaTime_R","1dDeltaTime_R");

  dir1dCsI_Energy = dirSummary->mkdir("1dCsI_Energy","1dCsI_Energy");
  dir1dCsI_Time = dirSummary->mkdir("1dCsI_Time","1dCsI_Time");



  //all the same plots for WW array
  dirWWSummary = new TDirectoryFile("WWSum", "WWSum"); //name, title
  dir1dWWFront_R = dirWWSummary->mkdir("1dWWFront_R","1dWWFront_R");
  dir1dWWBack_R = dirWWSummary->mkdir("1dWWBack_R","1dWWBack_R");
  dir1dWWDelta_R = dirWWSummary->mkdir("1dWWDelta_R","1dWWDelta_R");

  dir1dWWFront_cal = dirWWSummary->mkdir("1dWWFront_cal","1dWWFront_cal");
  dir1dWWBack_cal = dirWWSummary->mkdir("1dWWBack_cal","1dWWBack_cal");
  dir1dWWDelta_cal = dirWWSummary->mkdir("1dWWDelta_cal","1dWWDelta_cal");

  dir1dWWFrontTime_R = dirWWSummary->mkdir("1dWWFrontTime_R","1dWWFrontTime_R");
  dir1dWWBackTime_R = dirWWSummary->mkdir("1dWWBackTime_R","1dWWBackTime_R");
  dir1dWWDeltaTime_R = dirWWSummary->mkdir("1dWWDeltaTime_R","1dWWDeltaTime_R");


  //directory for DeltaE-E plots
  dirDEEplots = new TDirectoryFile("DEEplots","DEEplots");
  dirhitmaps = new TDirectoryFile("hitmaps","hitmaps");


  //director for all correlations and inv-mass
  dirInvMass = new TDirectoryFile("InvMass","InvMass");
  //make some sub directories to store each isotopes
  dir14O = dirInvMass->mkdir("14O","14O");

//  ^ directories
/////////////////////////////////////////////////////////////////////////
//  v summaries 


  //for calibrated spectra
  int Nbin = 5000;
  float Ecal_Emax = 50.0;
  float Delta_Emax = 16.0;
  float CsI_Emax = 30.0

  //defined in the header
  //E_boardnum = 8; dE_boardnum = 4; boardnum = 16; channum = 32;

  dirSummary->cd();
  //All Gobbi summaries
  //Energies, Raw+Calibrated
  sumFrontE_R = new TH2I("sumFrontE_R","",4*channum,0,4*channum,1024,0,8192);
  sumFrontE_R->SetOption("colz");
  sumBackE_R = new TH2I("sumBackE_R","",4*channum,0,4*channum,1024,0,8192);
  sumBackE_R->SetOption("colz");
  sumDeltaE_R = new TH2I("sumDeltaE_R","",4*channum,0,4*channum,1024,0,8192);
  sumDeltaE_R->SetOption("colz");

  sumFrontE_cal = new TH2I("sumFrontE_cal","",4*channum,0,4*channum,Nbin,0,Ecal_Emax);
  sumFrontE_cal->SetOption("colz");
  sumBackE_cal = new TH2I("sumBackE_cal","",4*channum,0,4*channum,Nbin,0,Ecal_Emax);
  sumBackE_cal->SetOption("colz");
  sumDeltaE_cal = new TH2I("sumDeltaE_cal","",4*channum,0,4*channum,Nbin,0,Ecal_Emax);
  sumDeltaE_cal->SetOption("colz");

  sumCsIE_R = new TH2I("sumCsIE_R","",16,0,16,1024,0,8192);
  sumCsIE_R->SetOption("colz");
  sumCsIE_cal = new TH2I("sumCsIE_cal","",16,0,16,Nbin,0,CsI_Emax);
  sumCsIE_cal->SetOption("colz");

  //Gobbi times
  sumFrontTime_R = new TH2I("sumFrontTime_R","",4*channum,0,4*channum,512,0,16383);
  sumFrontTime_R->SetOption("colz");
  sumFrontTime_cal = new TH2I("sumFrontTime_cal","",4*channum,0,4*channum,512,0,16383);
  sumFrontTime_cal->SetOption("colz");
  sumBackTime_R = new TH2I("sumBackTime_R","",4*channum,0,4*channum,512,0,16383);
  sumBackTime_R->SetOption("colz");
  sumBackTime_cal = new TH2I("sumBackTime_cal","",4*channum,0,4*channum,512,0,16383);
  sumBackTime_cal->SetOption("colz");
  sumDeltaTime_R = new TH2I("sumDeltaTime_R","",4*channum,0,4*channum,512,0,16383);
  sumDeltaTime_R->SetOption("colz");
  sumDeltaTime_cal = new TH2I("sumDeltaTime_cal","",4*channum,0,4*channum,512,0,16383);
  sumDeltaTime_cal->SetOption("colz");
  sumCsITime_R = new TH2I("sumCsITime_R","",16,0,16,512,0,16383);
  sumCsITime_R->SetOption("colz");
  sumCsITime_cal = new TH2I("sumCsITime_cal","",16,0,16,512,0,16383);
  sumCsITime_cal->SetOption("colz");


  //all WW summaries
  //Energies, Raw+Calibrated
  dirWWSummary->cd();
  sumWWFrontE_R = new TH2I("sumWWFrontE_R","",channum,0,channum,1024,0,8192);
  sumWWFrontE_R->SetOption("colz");
  sumWWBackE_R = new TH2I("sumWWBackE_R","",channum,0,channum,1024,0,8192);
  sumWWBackE_R->SetOption("colz");
  sumWWDeltaE_R = new TH2I("sumWWDeltaE_R","",channum,0,channum,1024,0,8192);
  sumWWDeltaE_R->SetOption("colz");

  sumWWFrontE_cal = new TH2I("sumWWFrontE_cal","",channum,0,channum,Nbin,0,Ecal_Emax);
  sumWWFrontE_cal->SetOption("colz");
  sumWWBackE_cal = new TH2I("sumWWBackE_cal","",channum,0,channum,Nbin,0,Ecal_Emax);
  sumWWBackE_cal->SetOption("colz");
  sumWWDeltaE_cal = new TH2I("sumWWDeltaE_cal","",channum,0,channum,Nbin,0,Ecal_Emax);
  sumWWDeltaE_cal->SetOption("colz");

  //WW times
  sumWWFrontTime_R = new TH2I("sumWWFrontTime_R","",channum,0,channum,512,0,16383);
  sumWWFrontTime_R->SetOption("colz");
  sumWWFrontTime_cal = new TH2I("sumWWFrontTime_cal","",channum,0,channum,512,0,16383);
  sumWWFrontTime_cal->SetOption("colz");
  sumWWBackTime_R = new TH2I("sumWWBackTime_R","",channum,0,channum,512,0,16383);
  sumWWBackTime_R->SetOption("colz");
  sumWWBackTime_cal = new TH2I("sumWWBackTime_cal","",channum,0,channum,512,0,16383);
  sumWWBackTime_cal->SetOption("colz");
  sumWWDeltaTime_R = new TH2I("sumWWDeltaTime_R","",channum,0,channum,512,0,16383);
  sumWWDeltaTime_R->SetOption("colz");
  sumWWDeltaTime_cal = new TH2I("sumWWDeltaTime_cal","",channum,0,channum,512,0,16383);
  sumWWDeltaTime_cal->SetOption("colz");


//  ^ summaries
/////////////////////////////////////////////////////////////////////////
//  v 1d spectra

  ostringstream name;

  //create all Gobbi 1d spectra
  for (int board_i=0; board_i<E_boardnum/2; board_i++)
  {
    for (int chan_i=0; chan_i<channum; chan_i++)
    {
      //individual Front Energy
      dir1dFrontE_R->cd();
      name.str("");
      name << "FrontE_R" << board_i << "_" << chan_i;
      FrontE_R[board_i][chan_i] = new TH1I(name.str().c_str(),"",2048,0,8192);

      dir1dFrontlowE_R->cd();
      name.str("");
      name << "FrontElow_R" << board_i << "_" << chan_i;
      FrontElow_R[board_i][chan_i] = new TH1I(name.str().c_str(),"",1024,0,4095);

      dir1dFrontTime_R->cd();
      name.str("");
      name << "FrontTime_R" << board_i << "_" << chan_i;
      FrontTime_R[board_i][chan_i] = new TH1I(name.str().c_str(),"",1024,0,16383);

      dir1dFrontE_cal->cd();
      name.str("");
      name << "FrontE_cal" << board_i << "_" << chan_i;
      FrontE_cal[board_i][chan_i] = new TH1I(name.str().c_str(),"",Nbin,5,Ecal_Emax);

      //individual Back Energy
      dir1dBackE_R->cd();
      name.str("");
      name << "BackE_R" << board_i << "_" << chan_i;
      BackE_R[board_i][chan_i] = new TH1I(name.str().c_str(),"",2048,0,8192);

      dir1dBacklowE_R->cd();
      name.str("");
      name << "BackElow_R" << board_i << "_" << chan_i;
      BackElow_R[board_i][chan_i] = new TH1I(name.str().c_str(),"",1024,0,4095);

      dir1dBackTime_R->cd();
      name.str("");
      name << "BackTime_R" << board_i << "_" << chan_i;
      BackTime_R[board_i][chan_i] = new TH1I(name.str().c_str(),"",1024,0,16383);

      dir1dBackE_cal->cd();
      name.str("");
      name << "BackE_cal" << board_i << "_" << chan_i;
      BackE_cal[board_i][chan_i] = new TH1I(name.str().c_str(),"",Nbin,0,Ecal_Emax);

      //individual DeltaE
      dir1dDeltaE_R->cd();
      name.str("");
      name << "DeltaE_R" << board_i << "_" << chan_i;
      DeltaE_R[board_i][chan_i] = new TH1I(name.str().c_str(),"",1024,0,4095);

      dir1dDeltalowE_R->cd();
      name.str("");
      name << "DeltaElow_R" << board_i << "_" << chan_i;
      DeltaElow_R[board_i][chan_i] = new TH1I(name.str().c_str(),"",1024,0,4095);

      dir1dDeltaTime_R->cd();
      name.str("");
      name << "DeltaTime_R" << board_i << "_" << chan_i;
      DeltaTime_R[board_i][chan_i] = new TH1I(name.str().c_str(),"",1024,0,16383);

      dir1dDeltaE_cal->cd();
      name.str("");
      name << "DeltaE_cal" << board_i << "_" << chan_i;
      DeltaE_cal[board_i][chan_i] = new TH1I(name.str().c_str(),"",Nbin,0,Delta_Emax);
    }
  }

  //create all the CsI 1d spectra
  for (int ichan=0; ichan<NCsI; ichan++)
  {
    dir1dCsI_Energy->cd();
    name.str("");
    name << "CsI_Energy_" << ichan << "R_unmatched";
    CsI_Energy_R_um[ichan] = new TH1I(name.str().c_str(),"",1024,0,4095);

    name.str("");
    name << "CsI_Energy_" << ichan << "R_matched";
    CsI_Energy_R[ichan] = new TH1I(name.str().c_str(),"",1024,0,4095);

    name.str("");
    name << "CsI_Energy_" << ichan << "cal";
    CsI_Energy_cal[ichan] = new TH1I(name.str().c_str(),"",Nbin,0,CsI_Emax);

    dir1dCsI_Time->cd();
    name.str("");
    name << "CsI_Time_" << ichan << "R_unmatched";
    CsI_Time_R_um[ichan] = new TH1I(name.str().c_str(),"",1024,0,4095);

    name.str("");
    name << "CsI_Time_" << ichan << "R_matched";
    CsI_Time_R[ichan] = new TH1I(name.str().c_str(),"",1024,0,4095);

    name.str("");
    name << "CsI_Time_" << ichan << "cal";
    CsI_Time_cal[ichan] = new TH1I(name.str().c_str(),"",Nbin,0,CsI_Emax);
  }













  //create all WW 1d Front spectra
  for (int chan_i=0; chan_i<channum; chan_i++)
  {
    //individual Front Energy
    dir1dWWFrontE_R->cd();
    name.str("");
    name << "WWFrontE_R" << chan_i;
    WWFrontE_R[chan_i] = new TH1I(name.str().c_str(),"",2048,0,8192);

    dir1dWWFrontlowE_R->cd();
    name.str("");
    name << "WWFrontElow_R" << chan_i;
    WWFrontElow_R[chan_i] = new TH1I(name.str().c_str(),"",1024,0,4095);

    dir1dWWFrontTime_R->cd();
    name.str("");
    name << "WWFrontTime_R" << chan_i;
    WWFrontTime_R[chan_i] = new TH1I(name.str().c_str(),"",1024,0,16383);

    dir1dWWFrontE_cal->cd();
    name.str("");
    name << "WWFrontE_cal" << chan_i;
    WWFrontE_cal[chan_i] = new TH1I(name.str().c_str(),"",Nbin,5,Ecal_Emax);

    //individual Back Energy
    dir1dWWBackE_R->cd();
    name.str("");
    name << "WWBackE_R" << chan_i;
    WWBackE_R[chan_i] = new TH1I(name.str().c_str(),"",2048,0,8192);

    dir1dWWBacklowE_R->cd();
    name.str("");
    name << "WWBackElow_R" << chan_i;
    WWBackElow_R[chan_i] = new TH1I(name.str().c_str(),"",1024,0,4095);

    dir1dWWBackTime_R->cd();
    name.str("");
    name << "WWBackTime_R" << chan_i;
    WWBackTime_R[chan_i] = new TH1I(name.str().c_str(),"",1024,0,16383);

    dir1dWWBackE_cal->cd();
    name.str("");
    name << "WWBackE_cal" << chan_i;
    WWBackE_cal[chan_i] = new TH1I(name.str().c_str(),"",Nbin,0,Ecal_Emax);

    //individual DeltaE
    dir1dWWDeltaE_R->cd();
    name.str("");
    name << "WWDeltaE_R" << chan_i;
    WWDeltaE_R[chan_i] = new TH1I(name.str().c_str(),"",1024,0,4095);

    dir1dWWDeltalowE_R->cd();
    name.str("");
    name << "WWDeltaElow_R" << chan_i;
    WWDeltaElow_R[chan_i] = new TH1I(name.str().c_str(),"",1024,0,4095);

    dir1dWWDeltaTime_R->cd();
    name.str("");
    name << "WWDeltaTime_R" << chan_i;
    WWDeltaTime_R[chan_i] = new TH1I(name.str().c_str(),"",1024,0,16383);

    dir1dWWDeltaE_cal->cd();
    name.str("");
    name << "WWDeltaE_cal" << chan_i;
    WWDeltaE_cal[chan_i] = new TH1I(name.str().c_str(),"",Nbin,0,Delta_Emax);
  }



















  //create all spectra based on quadrants
  dirDEEplots->cd();
  for (int quad=0; quad<4; quad++)
  {
    name.str("");
    name << "DEE" << quad;
    DEE[quad] = new TH2I(name.str().c_str(),"",500,0,80,800,0,22); //E is x, DE is y

    name.str("");
    name << "timediff" << quad;   
    timediff[quad] = new TH1I(name.str().c_str(),"",1000,-2000,2000);
  }


  dirhitmaps->cd();
  xyhitmap = new TH2I("xyhitmap","", 100,-10,10,100,-10,10);

  protonhitmap = new TH2I("protonhitmap","", 100,-10,10,100,-10,10);

  //Evstheta[0] = new TH2I("Evstheta0","",50,0,25,Nbin,0,Ecal_Emax);
  //Evstheta[1] = new TH2I("Evstheta1","",50,0,25,Nbin,0,Ecal_Emax);
  //Evstheta[2] = new TH2I("Evstheta2","",50,0,25,Nbin,0,Ecal_Emax);
  //Evstheta[3] = new TH2I("Evstheta3","",50,0,25,Nbin,0,Ecal_Emax);
  //Evstheta_all = new TH2I("Evstheta_all","",50,0,25,Nbin,0,Ecal_Emax);
  //Theta = new TH1I("Theta","",50,0,25);




  //you will likely get this channel so I left this as an example
  //TODO build out a bunch of these for all the inv-mass channels you want to look at

  //O14, likely inelastic scattering to p13N
  dir14O->cd();

  Erel_14O_p13N = new TH1I("Erel_14O_p13N","",250,0,10); //will give 40 keV bins
  Ex_14O_p13N = new TH1I("Ex_14O_p13N","",375,0,15);
  ThetaCM_14O_p13N = new TH1I("ThetaCM_14O_p13N","",200,0,10);
  VCM_14O_p13N = new TH1I("VCM_14O_p13N","",100,0,14);

  Erel_p13N_costhetaH = new TH2I("Erel_p13N_costhetaH","",100,0,8,25,-1,1);
}


//*********************************************
histo::~histo()
{
  file_read->Write();
  cout << "histo written" << endl;
  file_read->Close();
}

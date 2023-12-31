#include "det.h"

//Nicolas Dronchi 2023_07_10
//Class written to handle handle the collection of detectors. Here Gobbi is 4 dE-E-CsI
//telescopes. the 5th tele in gobbi is the WW stack of two W silicons.

//using namespace std;
using std::cout;
using std::endl;


det::det(histo * Histo1)
{
  Histo = Histo1;

  //nice spot for some measurements - Changed Targetdist and thickness to planned values
  Targetdist = 10; //cm //distance from target to front of Gobbi
  TargetThickness = 9.472;//mg/cm^2 for CD2 tar1

  Gobbi = new gobbi(Histo); //pass in histo pointer
  Gobbi->SetTarget(Targetdist, TargetThickness);

  //TODO no idea if I'm creating this correctly
  TDC = new TDC1190(3,20,128); //registers 3 hits, reference channel 33, 128 total channels
  CsIADC = new caen();
  SiADC = new HINP();
}

det::~det()
{
  cout << "start det destr" << endl;
  delete TDC;
  delete CsIADC;
  delete SiADC; //added in these deletes, not sure they should be here?

  delete Gobbi;
  cout << "end det destr" << endl;
}

bool det::unpack(unsigned short *point)
{
  //reset the Detectors class
  Gobbi->reset();

  bool stat = true;
  stat = SiADC->unpackSi_HINP4(point); //unpack all the HINP chips
  if (!stat)
  {
    cout << "Bad Si-HINP" << endl;
    return stat;
  }
  //the point was passed by reference and is a continuuation of the spot in data stream
  stat = CsIADC->read(point); //unpack all the HINP chips
  if (!stat)
  {
    cout << "Bad CsI" << endl;
    return stat;
  }
  //TODO it is unclear, I think the TDC point is at the end after the CsI ADC????
  unsigned short* endpoint = TDC->read(point); //unpack all the HINP chips


  for (int i=0; i<SiADC->NstripsRead; i++)
  {
    //check for these impossible event types, then print chip# and chan#
    if (SiADC->board[i] > 12 || SiADC->chan[i] >= Histo->channum)
    {
      cout << "ADC->NstripsRead " << SiADC->NstripsRead << endl;
      cout << "i " << i << endl;
      cout << "Board " << SiADC->board[i] << " and chan " << SiADC->chan[i];
      cout << " unpacked but not saved" << endl;
      return true;
    }

    float Energy = 0;
    float time = 0; //can be calibrated or shifted later

    //position in board tells you what you have and what quadrant you are in
    //All Gobbi SiE detectors
    if (SiADC->board[i] == 1 || SiADC->board[i] == 3 || SiADC->board[i] == 5 || SiADC->board[i] == 7)
    {
      int quad = (SiADC->board[i] - 1)/2; 
      Gobbi->addFrontEvent(quad, SiADC->chan[i], SiADC->high[i], SiADC->low[i], SiADC->time[i]);
    }
    if (SiADC->board[i] == 2 || SiADC->board[i] == 4 || SiADC->board[i] == 6 || SiADC->board[i] == 8)
    {
      int quad = (SiADC->board[i]/2)-1;
      Gobbi->addBackEvent(quad, SiADC->chan[i], SiADC->high[i], SiADC->low[i], SiADC->time[i]);
    }
    if (SiADC->board[i] == 9 || SiADC->board[i] == 10 || SiADC->board[i] == 11 || SiADC->board[i] == 12)
    {
      int quad = (SiADC->board[i]-9);
      Gobbi->addDeltaEvent(quad, SiADC->chan[i], SiADC->high[i], SiADC->low[i], SiADC->time[i]);
    }
    if (SiADC->board[i] == 13)
    {
      //all WW events are going into the 5th telescope of gobbi
      //The order has been changed back to 13: W E Front, 14: W E Back, 15: W dE, I felt that it was more intuitive and it will stay like this.
      Gobbi->addFrontEvent(4, SiADC->chan[i], SiADC->high[i], SiADC->low[i], SiADC->time[i]);
    }
    if (SiADC->board[i] == 14)
    {
      Gobbi->addBackEvent(4, SiADC->chan[i], SiADC->high[i], SiADC->low[i], SiADC->time[i]);
    }
    if (SiADC->board[i] == 15)
    {
      Gobbi->addDeltaEvent(4, SiADC->chan[i], SiADC->high[i], SiADC->low[i], SiADC->time[i]);
    }
  }
  //data is unpacked and stored into Silicon class at this point

  int NE = 0; //index for the current energy position
  for (int i=0; i<CsIADC->number; i++)
  {
    if (CsIADC->underflow[i]) continue;
    if (CsIADC->overflow[i]) continue;
    
    cout << "CsI " << CsIADC->channel[i] << " " << CsIADC->data[i] << endl;

    Gobbi->DataE[NE].id = CsIADC->channel[i];
    Gobbi->DataE[NE].ienergy = CsIADC->data[i];
    Gobbi->NE++;
  }

  int NT = 0; //index for the current time position
  for (int i=0; i<TDC->Ndata; i++) //TODO check this loop
  {
    cout << "TDC " << TDC->dataOut[i].channel << " " << TDC->dataOut[i].time << endl;

    Gobbi->DataT[NE].id = TDC->dataOut[i].channel;
    Gobbi->DataT[NE].itime = TDC->dataOut[i].time;
    Gobbi->NT++;
  }

  Gobbi->MatchCsIEnergyTime();

  //leave this part out early in the experiment, it causes a lot of troubles!!
  Gobbi->SiNeighbours();

  //this method is doing a lot here.
  //  1. matches up either E-CsI or dE-E events
  //  2. plots dE-E plots, and determines PID
  //  3. Does energy corrections for each telescope
  Gobbi->analyze();
  
  //hitmaps and other plots based on Pid
  for (int id=0;id<4;id++) 
  {
    for (int isol=0; isol<Gobbi->Telescope[id]->Nsolution; isol++)
    {
      float xpos = Gobbi->Telescope[id]->Solution[isol].Xpos;
      float ypos = Gobbi->Telescope[id]->Solution[isol].Ypos;
      //protons
      if (Gobbi->Telescope[id]->Solution[isol].iZ == 1 && Gobbi->Telescope[id]->Solution[isol].iA == 1)
      {
        Histo->protonhitmap->Fill(xpos, ypos);
      }
    }
  }
  //Added in WW position plot, new hit map called oxygenhitmap
  for (int isol=0; isol<Gobbi->Telescope[4]->Nsolution; isol++)
  {
    float xpos = Gobbi->Telescope[4]->Solution[isol].Xpos;
    float ypos = Gobbi->Telescope[4]->Solution[isol].Ypos;
      //protons
      if (Gobbi->Telescope[4]->Solution[isol].iZ == 8 && Gobbi->Telescope[4]->Solution[isol].iA == 14)
      {
        Histo->oxygenhitmap->Fill(xpos, ypos);
      }
  }

  //transfer Solutions in Gobbi and WW to Correl
  Correl.reset();
  int goodMult = 0;
  for (int id=0;id<5;id++) 
  {
    for (int isol=0; isol<Gobbi->Telescope[id]->Nsolution; isol++)
    {
      //only keep solutions that have a pid
      if(Gobbi->Telescope[id]->Solution[isol].ipid)
      {
        Correl.load(&Gobbi->Telescope[id]->Solution[isol]);
        goodMult++;
      }
    }
  }

  if (goodMult >= 2)
  {
    //list all functions to look for correlations here
    corr_14O();
    corr_15F();
  }

  return true;
}


void det::corr_14O()
{
  //14O -> p+13N
  if(Correl.proton.mult == 1 && Correl.N13.mult == 1)
  {
    float const Q14O = mass_14O - (mass_13N+mass_p);
    Correl.zeroMask();
    Correl.proton.mask[0]=1;
    Correl.N13.mask[0]=1;
    Correl.makeArray(1);

    float Erel_14O = Correl.findErel();
    float thetaCM = Correl.thetaCM;
    float Ex = Erel_14O - Q14O;

    Histo->Erel_14O_p13N->Fill(Erel_14O);
    Histo->Ex_14O_p13N->Fill(Ex);
    Histo->ThetaCM_14O_p13N->Fill(thetaCM*180./acos(-1));
    Histo->VCM_14O_p13N->Fill(Correl.velocityCM);
    Histo->Erel_p13N_costhetaH->Fill(Erel_14O,Correl.cos_thetaH);
  }
}
/*
void det::corr_15O()
{
  //15O - > There needs to be a decay channel you expect -ND
  if (Correl.O15.mult == 1)
  {
    //wut is this Qval? -ND
    float const Q15O = mass_14O + mass_9Be - (mass_15O+mass_8Be);
    Correl.zeroMask();
    Correl.NO.mask[0]=1;
    Correl.makeArray(1);

    float Erel_15O = Correl.findErel();
    float thetaCM = Correl.thetaCM;
    float Ex = Erel_15O - Q15O;

    Histo->Erel_14O_15O->Fill(Erel_15O);
    Histo->Ex_14O_15O->Fill(Ex);
    Histo->ThetaCM_14O_15O->Fill(thetaCM*180./acos(-1));
    Histo->VCM_14O_15O->Fill(Correl.velocityCM);
    Histo->Erel_15O_costhetaH->Fill(Erel_15O,Correl.cos_thetaH);
  }
}
*/


void det::corr_15F()
{
  //15F -> p + 14O
  if(Correl.proton.mult == 1 && Correl.O14.mult == 1)
  {
    float const Q15F = mass_15F - (mass_14O+mass_p);
    Correl.zeroMask();
    Correl.proton.mask[0]=1;
    Correl.O14.mask[0]=1;
    Correl.makeArray(1);

    float Erel_15F = Correl.findErel();
    float thetaCM = Correl.thetaCM;
    float Ex = Erel_15F - Q15F;

    Histo->Erel_15F_p14O->Fill(Erel_15F);
    Histo->Ex_15F_p14O->Fill(Ex);
    Histo->ThetaCM_15F_p14O->Fill(thetaCM*180./acos(-1));
    Histo->VCM_15F_p14O->Fill(Correl.velocityCM);
    Histo->Erel_p14O_costhetaH->Fill(Erel_15F,Correl.cos_thetaH);
  }
}

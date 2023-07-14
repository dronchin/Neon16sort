#include "det.h"

//Nicolas Dronchi 2023_07_10
//Class written to handle handle the collection of detectors. Here Gobbi is 4 dE-E-E
//telescopes. WWtele is a stack of two W silicons.

//using namespace std;
using std::cout;
using std::endl;


det::det(histo * Histo1)
{
  Histo = Histo1;

  //nice spot for some measurements
  Targetdist = 23.95; //cm //distance from target to front of Gobbi
  TargetThickness = 2.65;//mg/cm^2 for CD2 tar1


  Gobbi = new gobbi(Histo); //pass in histo pointer
  Gobbi->SetTarget(Targetdist, TargetThickness);

  WW = new Silicon();
}

det::~det()
{
  delete Gobbi;
  delete WW;
}

bool Det::unpack(unsigned short *point)
{
  //reset the Detectors class
  Gobbi->reset();
  WW->reset()

  bool stat = true;
  stat = ADC->unpackSi_HINP4(point); //unpack all the HINP chips
  if (!stat)
  {
    cout << "Bad hira" << endl;
    return stat;
  }

  for (int i=0; i<ADC->NstripsRead; i++)
  {
    //check for these impossible event types, then print chip# and chan#
    if (ADC->board[i] > 12 || ADC->chan[i] >= Histo->channum)
    {
      cout << "ADC->NstripsRead " << ADC->NstripsRead << endl;
      cout << "i " << i << endl;
      cout << "Board " << ADC->board[i] << " and chan " << ADC->chan[i];
      cout << " unpacked but not saved" << endl;
      return true;
    }

    float Energy = 0;
    float time = 0; //can be calibrated or shifted later

    //position in board tells you what you have and what quadrant you are in
    //All Gobbi SiE detectors
    if (ADC->board[i] == 1 || ADC->board[i] == 3 || ADC->board[i] == 5 || ADC->board[i] == 7)
    {
      int quad = (ADC->board[i] - 1)/2; 
      Gobbi->addFrontEvent(quad, ADC->chan[i], ADC->high[i], ADC->low[i], ADC->time[i]);
    }
    if (ADC->board[i] == 2 || ADC->board[i] == 4 || ADC->board[i] == 6 || ADC->board[i] == 8)
    {
      int quad = (ADC->board[i]/2)-1;
      Gobbi->addBackEvent(quad, ADC->chan[i], ADC->high[i], ADC->low[i], ADC->time[i]);
    }
    if (ADC->board[i] == 9 || ADC->board[i] == 10 || ADC->board[i] == 11 || ADC->board[i] == 12)
    {
      int quad = (ADC->board[i]-9);
      Gobbi->addDeltaEvent(quad, ADC->chan[i], ADC->high[i], ADC->low[i], ADC->time[i]);
    }
    if (ADC->board[i] == 13)
    {
      WW->addFrontEvent(ADC->chan[i], ADC->high[i], ADC->low[i], ADC->time[i]);
    }
    if (ADC->board[i] == 14)
    {
      WW->addBackEvent(ADC->chan[i], ADC->high[i], ADC->low[i], ADC->time[i]);
    }
    if (ADC->board[i] == 15)
    {
      WW->addDeltaEvent(ADC->chan[i], ADC->high[i], ADC->low[i], ADC->time[i]);
    }
  }
  //data is unpacked and stored into Silicon class at this point

  //leave this part out early in the experiment, it causes a lot of troubles!!
  Gobbi->SiNeighbours();
  WW->SiNeighbours();


/////////////////////////////////////////////////////////////////////////////////////


  //hitmaps and other plots based on Pid
  for (int id=0;id<4;id++) 
  {
    for (int isol=0; isol<Gobbi->Silicon[id]->Nsolution; isol++)
    {
      float xpos = Gobbi->Silicon[id]->Solution[isol].Xpos;
      float ypos = Gobbi->Silicon[id]->Solution[isol].Ypos;
      //protons
      if (Gobbi->Silicon[id]->Solution[isol].iZ == 1 && Gobbi->Silicon[id]->Solution[isol].iA == 1)
      {
        Histo->protonhitmap->Fill(xpos, ypos);
      }
      
    }
  }

  //transfer Solutions in Gobbi and WW to Correl
  Correl.reset();
  int goodMult = 0;
  for (int id=0;id<4;id++) 
  {
    for (int isol=0; isol<Gobbi->Silicon[id]->Nsolution; isol++)
    {
      //only keep solutions that have a pid
      if(Gobbi->Silicon[id]->Solution[isol].ipid)
      {
        Correl.load(&Gobbi->Silicon[id]->Solution[isol]);
        goodMult++;
      }
    }
  }
  for (int isol=0; isol < WW->Silicon->Nsolution; isol++)
  {
    //only keep solutions that have a pid
    if(WW->Silicon->Solution[isol].ipid)
    {
      Correl.load(&WW->Silicon->Solution[isol]);
      goodMult++;
    }
  }

  if (goodMult >= 2)
  {
    //list all functions to look for correlations here
    corr_14O()
  }

  return true;
}


void Gobbi::corr_14O()
{
  // p+13N
  if(Correl.proton.mult == 1 && Correl.N13.mult == 1)
  {
    float const Q14O = mass_alpha - (mass_p + mass_t);
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

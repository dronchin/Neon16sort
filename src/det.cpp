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


  gobbi = new Gobbi(Histo); //pass in histo pointer
  gobbi->SetTarget(Targetdist, TargetThickness);

  WW = new Tele(Histo); //maybe i want to seperate this here?
}

det::~det()
{
  delete gobbi;
  delete WW;
}

bool Det::unpack(unsigned short *point)
{
  //reset the Detectors class
  gobbi->reset();
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
  Gobbi->Neighbours();
  WW->Neighbours();


/////////////////////////////////////////////////////////////////////////////////////



  //look at all the information/multiplicities and determine how to match up the information
  for (int id=0;id<4;id++) 
  {
    //save comp time if the event is obvious
    if (Silicon[id]->Front.Nstore ==1 && Silicon[id]->Back.Nstore ==1 && Silicon[id]->Delta.Nstore ==1)
    {
      totMulti += Silicon[id]->simpleFront();
      //cout << "simple " << totMulti << endl;
    }
    else //if higher multiplicity then worry about picking the right one
    {    //this also handles the case where Nstore=0 for any of the chanels
      totMulti += Silicon[id]->multiHit();
      //cout << "multi " << totMulti << endl;
    }
  }

  //plot E vs dE bananas and hitmap of paired dE,E events
  for (int id=0;id<4;id++) 
  {
    //cout << "id " << id << "   Nsol " << Silicon[id]->Nsolution << endl;
    for (int isol=0;isol<Silicon[id]->Nsolution; isol++)
    {
      //fill in hitmap of gobbi
      Silicon[id]->position(isol); //calculates x,y pos, and lab angle

      //these events are all blocked by the beam donut, radius 1.9cm
      if (1.5 > sqrt( pow(Silicon[id]->Solution[isol].Xpos,2)+ pow(Silicon[id]->Solution[isol].Ypos,2)) )
        continue;

      //fill in dE-E plots to select particle type
      float Ener = Silicon[id]->Solution[isol].energy + Silicon[id]->Solution[isol].denergy*(1-cos(Silicon[id]->Solution[isol].theta));

      Histo->DEE[id]->Fill(Ener, Silicon[id]->Solution[isol].denergy*cos(Silicon[id]->Solution[isol].theta));

      Histo->xyhitmap->Fill(Silicon[id]->Solution[isol].Xpos, Silicon[id]->Solution[isol].Ypos);
 

    }
  }

  //calculate and determine particle identification PID in the silicon
  int Pidmulti = 0;
  for (int id=0;id<4;id++) 
  {
    Pidmulti += Silicon[id]->getPID();
  }

  //hitmaps and other plots based on Pid
  for (int id=0;id<4;id++) 
  {
    for (int isol=0; isol<Silicon[id]->Nsolution; isol++)
    {
      float xpos = Silicon[id]->Solution[isol].Xpos;
      float ypos = Silicon[id]->Solution[isol].Ypos;
      //protons
      if (Silicon[id]->Solution[isol].iZ == 1 && Silicon[id]->Solution[isol].iA == 1)
      {
        Histo->protonhitmap->Fill(xpos, ypos);
        Histo->dTime_proton->Fill(Silicon[id]->Solution[isol].timediff);
      }
      
    }
  }

  //calc sumEnergy,then account for Eloss in target, then set Ekin and momentum of solutions
  //Eloss files are loaded in silicon
  for (int id=0;id<4;id++) 
  {
    Silicon[id]->calcEloss();
  }

  //transfer Solution class to Correl
  Correl.reset();
  int goodMult = 0;
  for (int id=0;id<4;id++) 
  {
    for (int isol=0; isol<Silicon[id]->Nsolution; isol++)
    {
      //only keep solutions that have a pid
      if(Silicon[id]->Solution[isol].ipid)
      {
        Correl.load(&Silicon[id]->Solution[isol]);
        goodMult++;
      }
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

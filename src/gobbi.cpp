#include "gobbi.h"

//Nicolas Dronchi 2023_07_10
//Class written to handle all specifics of the gobbi array
//such as communicating with HINP, calibrations, Checking for charge sharing in neighbor
//calculating geometry

using namespace std;

gobbi::gobbi(histo * Histo1)
{
  Histo = Histo1;

  for (int id=0;id<4;id++)
  {
    Telescope[id] = new telescope();
    Telescope[id]->init(id); //tells Telescope what position it is in
  }

                        //Ntele, Nstrip, filename, polynomial order, weave strips?
  FrontEcal = new calibrate(4, Histo->channum, "Cal/GobbiFrontEcal.dat", 1, true);
  BackEcal = new calibrate(4, Histo->channum, "Cal/GobbiBackEcal.dat", 1, true);
  DeltaEcal = new calibrate(4, Histo->channum, "Cal/GobbiDeltaEcal.dat", 1, true);
  CsIEcal = new calibrate(4, 4, "Cal/GobbiDeltaEcal.dat", 1, false);

  FrontTimecal = new calibrate(4, Histo->channum, "Cal/GobbiFrontTimecal.dat",1, false);
  BackTimecal = new calibrate(4, Histo->channum, "Cal/GobbiBackTimecal.dat",1, false);  
  DeltaTimecal = new calibrate(4, Histo->channum, "Cal/GobbiDeltaTimecal.dat",1, false);
  CsITimecal = new calibrate(4, 4, "Cal/GobbiDeltaTimecal.dat", 1, false);
}

gobbi::~gobbi()
{
  delete FrontEcal;
  delete BackEcal;
  delete DeltaEcal;
  delete CsIEcal;
  delete FrontTimecal;
  delete BackTimecal;
  delete DeltaTimecal;
  delete CsITimecal;
  //delete[] Telescope; //not needed as it is in automatic memory, didn't call with new
}

void gobbi::SetTarget(double Targetdist, float TargetThickness)
{
  for (int id=0;id<4;id++){ Telescope[id]->SetTarget(Targetdist, TargetThickness); }
}
void gobbi::reset()
{
  //reset the Telescope class
  for (int i=0;i<4;i++){ Telescope[i]->reset();}
  //make sure to reset the CsI here as well, whatever they end up looking like
}

void gobbi::addFrontEvent(int quad, unsigned short chan, unsigned short high, 
                                    unsigned short low, unsigned short time)
{
  //Use calibration to get Energy and fill elist class in Telescope
  float Energy = FrontEcal->getEnergy(quad, chan, high);
  float time = FrontTimecal->getTime(quad, chan, time);

  //good spot to fill in all of the summary and chan spectrums
  Histo->sumFrontE_R->Fill(quad*Histo->channum + chan, high);
  Histo->sumFrontTime_R->Fill(quad*Histo->channum + chan, time);
  Histo->sumFrontE_cal->Fill(quad*Histo->channum + chan, Energy);
  Histo->sumFrontTime_cal->Fill(quad*Histo->channum + chan, time);

  Histo->FrontE_R[quad][chan]->Fill(high);
  Histo->FrontElow_R[quad][chan]->Fill(low);
  Histo->FrontTime_R[quad][chan]->Fill(time);
  Histo->FrontE_cal[quad][chan]->Fill(Energy);      

  //TODO this is a good spot to throw an if statement and make software thresholds
  //if (Energy > .5)
  Telescope[quad]->Front.Add(chan, Energy, low, high, time);
  Telescope[quad]->multFront++;
}

void gobbi::addBackEvent(int quad, unsigned short chan, unsigned short high, 
                                   unsigned short low, unsigned short time)
{
  //Use calibration to get Energy and fill elist class in Telescope
  float Energy = BackEcal->getEnergy(quad, chan, high);
  float time = BackTimecal->getTime(quad, chan, time);

  //good spot to fill in all of the summary and chan spectrums
  Histo->sumBackE_R->Fill(quad*Histo->channum + chan, high);
  Histo->sumBackTime_R->Fill(quad*Histo->channum + chan, time);
  Histo->sumBackE_cal->Fill(quad*Histo->channum + chan, Energy);
  Histo->sumBackTime_cal->Fill(quad*Histo->channum + chan, time);

  Histo->BackE_R[quad][chan]->Fill(high);
  Histo->BackElow_R[quad][chan]->Fill(low);
  Histo->BackTime_R[quad][chan]->Fill(time);
  Histo->BackE_cal[quad][chan]->Fill(Energy);      

  //TODO this is a good spot to throw an if statement and make software thresholds
  //if (Energy > .5)
  Telescope[quad]->Back.Add(chan, Energy, low, high, time);
  Telescope[quad]->multBack++;
}

void gobbi::addDeltaEvent(int quad, unsigned short chan, unsigned short high, 
                                    unsigned short low, unsigned short time)
{
  //Use calibration to get Energy and fill elist class in Telescope
  float Energy = DeltaEcal->getEnergy(quad, chan, high);
  float time = DeltaTimecal->getTime(quad, chan, time);

  //good spot to fill in all of the summary and chan spectrums
  Histo->sumDeltaE_R->Fill(quad*Histo->channum + chan, high);
  Histo->sumDeltaTime_R->Fill(quad*Histo->channum + chan, time);
  Histo->sumDeltaE_cal->Fill(quad*Histo->channum + chan, Energy);
  Histo->sumDeltaTime_cal->Fill(quad*Histo->channum + chan, time);

  Histo->DeltaE_R[quad][chan]->Fill(high);
  Histo->DeltaElow_R[quad][chan]->Fill(low);
  Histo->DeltaTime_R[quad][chan]->Fill(time);
  Histo->DeltaE_cal[quad][chan]->Fill(Energy);      

  //TODO this is a good spot to throw an if statement and make software thresholds
  //if (Energy > .5)
  Telescope[quad]->Delta.Add(chan, Energy, low, high, time);
  Telescope[quad]->multDelta++;
}

void gobbi::addCsIEvent(int quad, unsigned short chan, unsigned short high, 
                                  unsigned short low, unsigned short time)
{
  //TODO major TODO
  //this part is going to need some testing and work. 
  //I don't know and can't anticipate what it should look like
  return
}



void gobbi::SiNeigbours()
{
  //When a charged particle moves through Si, there is cross talk between neighbouring
  //strips. Generally the signal is proportional to the total signal in the Si.
  for (int id=0;id<4;id++) 
  {
    Telescope[id]->Front.Neighbours(id);
    Telescope[id]->Back.Neighbours(id);
    Telescope[id]->Delta.Neighbours(id);
  }
}

int gobbi::matchTele()
{
  //TODO fix all of this! need to match CsI info here

  //look at all the information/multiplicities and determine how to match up 
  //the information
  multiSiSi = 0;
  multiSiSiCsI = 0;
  int Nmatch = 0;
  for (int id=0;id<4;id++) 
  {
    //look for a full stack Si-Si-CsI hit or just the Si-Si hit
    if (Telescope[id]->CsI.Nstore >= 1 && Telescope[id]->Front.Nstore >=1 && Telescope[id]->Back.Nstore >=1 && Telescope[id]->Delta.Nstore >=1)
    {
      //look for the simple case first
      if (Telescope[id]->CsI.Nstore == 1 && Telescope[id]->Front.Nstore ==1 && Telescope[id]->Back.Nstore ==1 && Telescope[id]->Delta.Nstore ==1)
      {
        Nmatch = Telescope[id]->simpleSiSiCsI();
        NsimpleSiSiCsI += Nmatch;
        multiSiSiCsI += Nmatch;
      }
      else //then look at the multihit case
      {
        Nmatch = Telescope[id]->multiHitSiSiCsI();
        NmultiSiSiCsI += Nmatch;
        multiSiSiCsI += Nmatch;
      }
    }
    //purposely coded to exclude events that likely puched through second Si
    //otherwise could cause a lot of noise in Si-Si DEE plot 
    else if (Telescope[id]->Front.Nstore >=1 && Telescope[id]->Back.Nstore >=1 && Telescope[id]->Delta.Nstore >=1)
    {
      //save comp time if the event is obvious
      if (Telescope[id]->Front.Nstore ==1 && Telescope[id]->Back.Nstore ==1 && Telescope[id]->Delta.Nstore ==1)
      {
        Nmatch = Telescope[id]->simpleFront();
        NsimpleSiSi += Nmatch;
        multiSiSi += Nmatch;
      }
      else //if higher multiplicity then worry about picking the right one
      {
        Nmatch = Telescope[id]->multiHit();
        NmultiSiSi += Nmatch;
        multiSiSi += Nmatch;
      }
    }
  }

  //plot E vs dE bananas and hitmap of paired dE,E events
  for (int id=0;id<4;id++) 
  {
    for (int isol=0;isol<Telescope[id]->Nsolution; isol++)
    {
      //fill in hitmap of gobbi
      Telescope[id]->position(isol); //calculates x,y pos, and lab angle
      Histo->xyhitmap->Fill(Telescope[id]->Solution[isol].Xpos, 
                            Telescope[id]->Solution[isol].Ypos);

      bool isSiCsI = Telescope[id]->isSiCsI()

      //fill in dE-E plots to select particle type
      //TODO there is a different correction on the de when thick Si is dE, probably also required for E now
      if (isSiCsI)
      {
        float Ener = Telescope[id]->Solution[isol].energy + 
            Telescope[id]->Solution[isol].denergy*(1-cos(Telescope[id]->Solution[isol].theta));
      
        Histo->DEE_CsI[id]->Fill(Ener, Telescope[id]->Solution[isol].denergy*
                                    cos(Telescope[id]->Solution[isol].theta));
      }
      else
      {
        float Ener = Telescope[id]->Solution[isol].energy + 
            Telescope[id]->Solution[isol].denergy*(1-cos(Telescope[id]->Solution[isol].theta));

        Histo->DEE[id]->Fill(Ener, Telescope[id]->Solution[isol].denergy*
                                    cos(Telescope[id]->Solution[isol].theta));
      }
      
    }
  }

  //calculate and determine particle identification PID in the Telescope
  int Pidmulti = 0;
  for (int id=0;id<4;id++) 
  {
    Pidmulti += Telescope[id]->getPID();
  }

  //calc sumEnergy,then account for Eloss in target, then set Ekin and momentum of solutions
  //Eloss files are loaded in Telescope
  for (int id=0;id<4;id++) 
  {
    Telescope[id]->calcEloss();
  }

  return multiSiSi + multiSiSiCsI;
}

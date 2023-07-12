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
    Silicon[id] = new silicon();
    Silicon[id]->init(id); //tells Silicon what position it is in
  }

  FrontEcal = new calibrate(4, Histo->channum, "Cal/GobbiFrontEcal.dat", 1, true);
  BackEcal = new calibrate(4, Histo->channum, "Cal/GobbiBackEcal.dat", 1, true);
  DeltaEcal = new calibrate(4, Histo->channum, "Cal/GobbiDeltaEcal.dat", 1, true);

  FrontTimecal = new calibrate(4, Histo->channum, "Cal/GobbiFrontTimecal.dat",1, false);
  BackTimecal = new calibrate(4, Histo->channum, "Cal/GobbiBackTimecal.dat",1, false);  
  DeltaTimecal = new calibrate(4, Histo->channum, "Cal/GobbiDeltaTimecal.dat",1, false);
}

gobbi::~gobbi()
{
  delete FrontEcal;
  delete BackEcal;
  delete DeltaEcal;
  delete FrontTimecal;
  delete BackTimecal;
  delete DeltaTimecal;
  //delete[] Silicon; //not needed as it is in automatic memory, didn't call with new
}

void gobbi::SetTarget(double Targetdist, float TargetThickness)
{
  for (int id=0;id<4;id++)
  {
    Silicon[id]->SetTargetDistance(Targetdist);
    Silicon[id]->SetTargetThickness(TargetThickness);
  }
}

void gobbi::addFrontEvent(int quad, unsigned short chan, unsigned short high, 
                                    unsigned short low, unsigned short time)
{
  //Use calibration to get Energy and fill elist class in silicon
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
  Silicon[quad]->Front.Add(chan, Energy, low, high, time);
  Silicon[quad]->multFront++;
}

void gobbi::addBackEvent(int quad, unsigned short chan, unsigned short high, 
                                   unsigned short low, unsigned short time)
{
  //Use calibration to get Energy and fill elist class in silicon
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
  Silicon[quad]->Back.Add(chan, Energy, low, high, time);
  Silicon[quad]->multBack++;
}

void gobbi::addDeltaEvent(int quad, unsigned short chan, unsigned short high, 
                                    unsigned short low, unsigned short time)
{
  //Use calibration to get Energy and fill elist class in silicon
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
  Silicon[quad]->Delta.Add(chan, Energy, low, high, time);
  Silicon[quad]->multDelta++;
}

void gobbi::addCsIEvent(int quad, unsigned short chan, unsigned short high, 
                                  unsigned short low, unsigned short time)
{
  //TODO major TODO
  //this part is going to need some testing and work. 
  //I don't know and can't anticipate what it should look like
  return
}

void gobbi::reset()
{
  //reset the Silicon class
  for (int i=0;i<4;i++){ Silicon[i]->reset();}
  //make sure to reset the CsI here as well, whatever they end up looking like
}

void gobbi::SiNeigbours()
{
  //When a charged particle moves through Si, there is cross talk between neighbouring
  //strips. Generally the signal is proportional to the total signal in the Si.
  for (int id=0;id<4;id++) 
  {
    Silicon[id]->Front.Neighbours(id);
    Silicon[id]->Back.Neighbours(id);
    Silicon[id]->Delta.Neighbours(id);
  }
}

int gobbi::matchTele()
{
  //TODO fix all of this! need to match CsI info here

  //look at all the information/multiplicities and determine how to match up 
  //the information
  totMulti = 0;
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
    for (int isol=0;isol<Silicon[id]->Nsolution; isol++)
    {
      //fill in hitmap of gobbi
      Silicon[id]->position(isol); //calculates x,y pos, and lab angle

      //fill in dE-E plots to select particle type
      //TODO there is a correction on the de for angle, probably also required for E now
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

  //calc sumEnergy,then account for Eloss in target, then set Ekin and momentum of solutions
  //Eloss files are loaded in silicon
  for (int id=0;id<4;id++) 
  {
    Silicon[id]->calcEloss();
  }

  return totMulti;
}

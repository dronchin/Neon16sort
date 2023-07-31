#include "gobbi.h"

//Nicolas Dronchi 2023_07_10
//Class written to handle all specifics of the gobbi array
//such as communicating with HINP, calibrations, Checking for charge sharing in neighbor
//calculating geometry
//                 ,,__
//        ..  ..   / o._)                   .---.
//       /--'/--\  \-'||        .----.    .'     '.
//      /        \_/ / |      .'      '..'         '-.
//    .'\  \__\  __.'.'     .'          e-._
//      )\ |  )\ |      _.'
//     // \\ // \\
//    ||_  \\|_  \\_             (Wait Gobbi the person, not Gobi desert)
//    '--' '--'' '--'


using namespace std;

gobbi::gobbi(histo * Histo1)
{
  Histo = Histo1;

  for (int id=0;id<5;id++)
  {
    Telescope[id] = new telescope();
    Telescope[id]->init(id); //tells Telescope what position it is in
    Telescope[id]->load(0,0,15,15,   
                        16,16,31,31,
                        0,15,15,0,
                        16,31,31,16);  //load stip limts for each CsI crystal
  }
                        //Ntele, Nstrip, filename, polynomial order, weave strips?
  
  //TODO
  //WW does not weave channels so the last true doesn't work for them?
  //actually not convinced the W doesn't weave strip numbers. really needs to be
  //figured out.

  FrontEcal = new calibrate(5, Histo->channum, "Cal/GobbiFrontEcal.dat", 1, true);
  BackEcal = new calibrate(5, Histo->channum, "Cal/GobbiBackEcal.dat", 1, true);
  DeltaEcal = new calibrate(5, Histo->channum, "Cal/GobbiDeltaEcal.dat", 1, true);
  CsIEcal = new calibrate(4, 4, "Cal/GobbiDeltaEcal.dat", 1, false);

  FrontTimecal = new calibrate(5, Histo->channum, "Cal/GobbiFrontTimecal.dat",1, false);
  BackTimecal = new calibrate(5, Histo->channum, "Cal/GobbiBackTimecal.dat",1, false);  
  DeltaTimecal = new calibrate(5, Histo->channum, "Cal/GobbiDeltaTimecal.dat",1, false);
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
  //WW is 20 cm from gobbi -JP?
  Telescope[4]->SetTarget(Targetdist+20, TargetThickness); 
}
void gobbi::reset()
{
  //reset the Telescope class
  for (int i=0;i<5;i++){ Telescope[i]->reset();}
  //make sure to reset the CsI here as well, whatever they end up looking like
}

void gobbi::addFrontEvent(int quad, unsigned short chan, unsigned short high, 
                                    unsigned short low, unsigned short timeR)
{
  //Use calibration to get Energy and fill elist class in Telescope
  float Energy = FrontEcal->getEnergy(quad, chan, high);
  float time = FrontTimecal->getTime(quad, chan, timeR);

  //good spot to fill in all of the summary and chan spectrums
  Histo->sumFrontE_R->Fill(quad*Histo->channum + chan, high);
  Histo->sumFrontTime_R->Fill(quad*Histo->channum + chan, timeR);
  Histo->sumFrontE_cal->Fill(quad*Histo->channum + chan, Energy);
  Histo->sumFrontTime_cal->Fill(quad*Histo->channum + chan, time);

  Histo->FrontE_R[quad][chan]->Fill(high);
  Histo->FrontElow_R[quad][chan]->Fill(low);
  Histo->FrontTime_R[quad][chan]->Fill(timeR);
  Histo->FrontE_cal[quad][chan]->Fill(Energy);      

  //TODO this is a good spot to throw an if statement and make software thresholds
  //if (Energy > .5)
  Telescope[quad]->Front.Add(chan, Energy, low, high, time);
  Telescope[quad]->multFront++;
}

void gobbi::addBackEvent(int quad, unsigned short chan, unsigned short high, 
                                   unsigned short low, unsigned short timeR)
{
  //Use calibration to get Energy and fill elist class in Telescope
  float Energy = BackEcal->getEnergy(quad, chan, high);
  float time = BackTimecal->getTime(quad, chan, time);

  //good spot to fill in all of the summary and chan spectrums
  Histo->sumBackE_R->Fill(quad*Histo->channum + chan, high);
  Histo->sumBackTime_R->Fill(quad*Histo->channum + chan, timeR);
  Histo->sumBackE_cal->Fill(quad*Histo->channum + chan, Energy);
  Histo->sumBackTime_cal->Fill(quad*Histo->channum + chan, time);

  Histo->BackE_R[quad][chan]->Fill(high);
  Histo->BackElow_R[quad][chan]->Fill(low);
  Histo->BackTime_R[quad][chan]->Fill(timeR);
  Histo->BackE_cal[quad][chan]->Fill(Energy);      

  //TODO this is a good spot to throw an if statement and make software thresholds
  //if (Energy > .5)
  Telescope[quad]->Back.Add(chan, Energy, low, high, time);
  Telescope[quad]->multBack++;
}

void gobbi::addDeltaEvent(int quad, unsigned short chan, unsigned short high, 
                                    unsigned short low, unsigned short timeR)
{
  //Use calibration to get Energy and fill elist class in Telescope
  float Energy = DeltaEcal->getEnergy(quad, chan, high);
  float time = DeltaTimecal->getTime(quad, chan, time);

  //good spot to fill in all of the summary and chan spectrums
  Histo->sumDeltaE_R->Fill(quad*Histo->channum + chan, high);
  Histo->sumDeltaTime_R->Fill(quad*Histo->channum + chan, timeR);
  Histo->sumDeltaE_cal->Fill(quad*Histo->channum + chan, Energy);
  Histo->sumDeltaTime_cal->Fill(quad*Histo->channum + chan, time);

  Histo->DeltaE_R[quad][chan]->Fill(high);
  Histo->DeltaElow_R[quad][chan]->Fill(low);
  Histo->DeltaTime_R[quad][chan]->Fill(timeR);
  Histo->DeltaE_cal[quad][chan]->Fill(Energy);      

  //TODO this is a good spot to throw an if statement and make software thresholds
  //if (Energy > .5)
  Telescope[quad]->Delta.Add(chan, Energy, low, high, time);
  Telescope[quad]->multDelta++;
}


void gobbi::MatchCsIEnergyTime()
{
  int Nfound = 0;
  int Nnotfound = 0;

  //plot unmatched energy and times
  for (int ie=0;ie<NE;ie++)
  {
    Histo->CsI_Energy_R_um[DataE[ie].id]->Fill(DataE[ie].ienergy);
  }
  for (int it=0;it<NT;it++)
  {
    Histo->CsI_Time_R_um[DataT[it].id]->Fill(DataT[it].itime);
  }

  // match up energies to times
  for (int ie=0;ie<NE;ie++)
  {
    DataE[ie].itime = -1;
    bool found = false;
    for (int it=0;it<NT;it++)
    {
      if (DataE[ie].id == DataT[it].id)           //we have matched
      {
        found  = true;
        DataE[ie].itime = DataT[it].itime;

        Nfound++;
        
        //add event to the right telescope elsit of csi 
        //TODO take out energy calibration here, Calibrations need to be for each CsI crystal
        //and is also dependent on PID in CsI. -ND
        int id = DataE[ie].id;
        if (id <4)
        {
          int quad = 0;
          int chan = DataE[ie].id;
          //CsI energy calibration here!
          DataE[ie].energy = CsIEcal->getEnergy(quad, chan, DataE[ie].ienergy);
          DataE[ie].time = CsITimecal->getTime(quad, chan, DataE[ie].itime);

          Telescope[quad]->CsI.Add(chan, DataE[ie].energy, 0, 
                                   DataE[ie].ienergy, DataE[ie].time);
          Telescope[quad]->multCsI++;
        }
        else if(id < 8 and id >= 4)
        {
          int quad = 1;
          int chan = DataE[ie].id - 4; //shift id so it is 0,1,2,3
          DataE[ie].energy = CsIEcal->getEnergy(quad, chan, DataE[ie].ienergy);
          DataE[ie].time = CsITimecal->getTime(quad, chan, DataE[ie].itime);

          Telescope[quad]->CsI.Add(chan, DataE[ie].energy, 0, 
                                   DataE[ie].ienergy, DataE[ie].time);
          Telescope[quad]->multCsI++;
        }
        else if(id < 12 and id >= 8)
        {
          int quad = 2;
          int chan = DataE[ie].id - 8; //shift id so it is 0,1,2,3
          DataE[ie].energy = CsIEcal->getEnergy(quad, chan, DataE[ie].ienergy);
          DataE[ie].time = CsITimecal->getTime(quad, chan, DataE[ie].itime);

          Telescope[quad]->CsI.Add(chan, DataE[ie].energy, 0, 
                                   DataE[ie].ienergy, DataE[ie].time);
          Telescope[quad]->multCsI++;
        }
        else if(id < 16 and id >= 12)
        {
          int quad = 3;
          int chan = DataE[ie].id - 12; //shift id so it is 0,1,2,3
          DataE[ie].energy = CsIEcal->getEnergy(quad, chan, DataE[ie].ienergy);
          DataE[ie].time = CsITimecal->getTime(quad, chan, DataE[ie].itime);

          Telescope[quad]->CsI.Add(chan, DataE[ie].energy, 0, 
                                   DataE[ie].ienergy, DataE[ie].time);
          Telescope[quad]->multCsI++;
        }
        else
        {
          //we have an issue. what did YOU do?
          cout << "check the id of CsI coming in" << endl;
        }

        Histo->CsI_Energy_R[id]->Fill(DataE[ie].ienergy);
        Histo->CsI_Energy_cal[id]->Fill(DataE[ie].energy);
        Histo->CsI_Time_R[id]->Fill(DataE[ie].itime);
        Histo->CsI_Time_cal[id]->Fill(DataE[ie].time);

      }
    }
  }
}


//   _.+._
// (^\/^\/^)
//  \@*@*@/
//  {_____} Just as the queen would prefer to spell it
void gobbi::SiNeighbours()
{
  //When a charged particle moves through Si, there is cross talk between neighbouring
  //strips. Generally the signal is proportional to the total signal in the Si.
  for (int id=0;id<5;id++) 
  {
    Telescope[id]->Front.Neighbours(id);
    Telescope[id]->Back.Neighbours(id);
    Telescope[id]->Delta.Neighbours(id);
  }
}

int gobbi::analyze()
{
  //TODO fix all of this! need to match CsI info here

  //look at all the information/multiplicities and determine how to match up 
  //the information
  multidEE = 0;
  multiECsI = 0;
  int Nmatch = 0;
  for (int id=0;id<5;id++) 
  {

    //WARNING: this is only in for testing alpha calibrations. Comment this out
    //when you are taking real data
    if (Telescope[id]->Front.Nstore ==1 && Telescope[id]->Back.Nstore ==1)
    {
      int testhit = Telescope[id]->testingHitE();
      Telescope[id]->position(9);
      Histo->testinghitmap->Fill(Telescope[id]->Solution[9].Xpos, Telescope[id]->Solution[9].Ypos);
    }
    


    Telescope[id]->Nsolution = 0;
    //If there is any CsI info, use E-CsI hit scheme
    if (id < 4 && Telescope[id]->CsI.Nstore >= 1)
    {
      //look for the simple case first
      if (Telescope[id]->CsI.Nstore == 1 && Telescope[id]->Front.Nstore ==1 && Telescope[id]->Back.Nstore ==1)
      {
        Nmatch = Telescope[id]->simpleECsI();
        NsimpleECsI += Nmatch; //increase total count
        multiECsI += Nmatch;   //increase current count
      }
      else //then look at the multihit case
      {
        Nmatch = Telescope[id]->multiHitECsI();
        NmultiECsI += Nmatch;
        multiECsI += Nmatch;
      }
    }
    //Need to be careful with puch-through events here
    //otherwise could cause a lot of noise in Si-Si dEE plot
    //warning, events that have a ECsI and a dEE in the same telescope are not saved here.
    if (Telescope[id]->Front.Nstore >=1 && Telescope[id]->Back.Nstore >=1 && Telescope[id]->Delta.Nstore >=1 && Telescope[id]->CsI.Nstore < 1)
    {
      //save comp time if the event is obvious
      if (Telescope[id]->Front.Nstore ==1 && Telescope[id]->Back.Nstore ==1 && Telescope[id]->Delta.Nstore ==1)
      {
        Nmatch = Telescope[id]->simpledEE();
        NsimpledEE += Nmatch;
        multidEE += Nmatch;
      }
      else //if higher multiplicity then worry about picking the right one
      {
        Nmatch = Telescope[id]->multiHitdEE();
        NmultidEE += Nmatch;
        multidEE += Nmatch;
      }
    }
  }

  //plot E vs dE bananas and hitmap of paired dE,E events
  for (int id=0;id<5;id++) 
  {
    for (int isol=0;isol<Telescope[id]->Nsolution; isol++)
    {
      //fill in hitmap of gobbi
      Telescope[id]->position(isol); //calculates x,y pos, and lab angle
      Histo->xyhitmap->Fill(Telescope[id]->Solution[isol].Xpos, 
                            Telescope[id]->Solution[isol].Ypos);

      bool isSiCsI = Telescope[id]->Solution[isol].isSiCsI;

      //fill in dE-E plots to select particle type
      if (isSiCsI)
      {
        //float Ener = Telescope[id]->Solution[isol].energy + 
        //  Telescope[id]->Solution[isol].denergy*(1-cos(Telescope[id]->Solution[isol].theta));
        float Ener = Telescope[id]->Solution[isol].energyR;
        int icsi = Telescope[id]->Solution[isol].iCsI;
        Histo->DEE_CsI[id][icsi]->Fill(Ener, Telescope[id]->Solution[isol].denergy*
                                            cos(Telescope[id]->Solution[isol].theta));
        Histo->timediff_CsI[id][icsi]->Fill(Telescope[id]->Solution[isol].timediff);
      }
      else
      {
        float Ener = Telescope[id]->Solution[isol].energy + 
            Telescope[id]->Solution[isol].denergy*(1-cos(Telescope[id]->Solution[isol].theta));

        Histo->DEE[id]->Fill(Ener, Telescope[id]->Solution[isol].denergy*
                                    cos(Telescope[id]->Solution[isol].theta));
        Histo->timediff[id]->Fill(Telescope[id]->Solution[isol].timediff);
      }
    }
  }

  //calculate and determine particle identification PID in the Telescope
  int Pidmulti = 0;
  for (int id=0;id<5;id++) 
  {
    Pidmulti += Telescope[id]->getPID();
  }

  //calc sumEnergy,then account for Eloss in target, then set Ekin and momentum of solutions
  //Eloss files are loaded in Telescope
  for (int id=0;id<5;id++) 
  {
    Telescope[id]->calcEloss();
  }

  return multidEE + multiECsI;
}



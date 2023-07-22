//class dealing with a single Hira Si telescope

#include "silicon.h"

using namespace std;

//**********************************************************
  //constructor
silicon::silicon(histo * Histo1)
{
  Histo = Histo1;

  SiWidth = 4.95; //cm
  //frame for a micron W 53.78 mm x 53.78 mm 
  //with an active area of 49.50 mm x 49.50 mm. 

  //WW is at 0-deg
  Xcenter = 0.;
  Ycenter = 0.;

  id = 0;
  ostringstream outstring;  
  outstring << "WWpid";

  Pid = new pid(outstring.str());

  //switch which loss file is used depending on the target.
  losses = new CLosses(6,"_CD2.loss");

  FrontEcal = new calibrate(1, Histo->channum, "Cal/WWFrontEcal.dat", 1, true);
  BackEcal = new calibrate(1, Histo->channum, "Cal/WWBackEcal.dat", 1, true);
  DeltaEcal = new calibrate(1, Histo->channum, "Cal/WWDeltaEcal.dat", 1, true);

  FrontTimecal = new calibrate(1, Histo->channum, "Cal/WWFrontTimecal.dat", 1, true);
  BackTimecal = new calibrate(1, Histo->channum, "Cal/WWBackTimecal.dat", 1, true);
  DeltaTimecal = new calibrate(1, Histo->channum, "Cal/WWDeltaTimecal.dat", 1, true);

  Ran = new TRandom();
}

//destructor
silicon::~silicon()
{
  delete losses;
  delete Ran;
  delete Pid;
}

void silicon::SetTarget(double Targetdist, float thick)
{
  for (int i=0;i<10;i++) Solution[i].SetTargetDistance(Targetdist);
  TargetThickness = thick;
}

void silicon::reset()
{
  multFront = 0;
  multBack = 0;
  multDelta = 0;

  Front.reset();
  Back.reset();
  Delta.reset();

  for (int i=0; i<10; i++){ Solution[i].reset();}

  Nsolution = 0;
}




void silicon::addFrontEvent(unsigned short chan, unsigned short high, 
                            unsigned short low, unsigned short time)
{
  //Use calibration to get Energy and fill elist class in Telescope
  float Energy = FrontEcal->getEnergy(0, chan, high);
  float time = FrontTimecal->getTime(0, chan, time);

  //good spot to fill in all of the summary and chan spectrums
  Histo->sumWWFrontE_R->Fill(chan, high);
  Histo->sumWWFrontTime_R->Fill(chan, time);
  Histo->sumWWFrontE_cal->Fill(chan, Energy);
  Histo->sumWWFrontTime_cal->Fill(chan, time);

  Histo->WWFrontE_R[chan]->Fill(high);
  Histo->WWFrontElow_R[chan]->Fill(low);
  Histo->WWFrontTime_R[chan]->Fill(time);
  Histo->WWFrontE_cal[chan]->Fill(Energy);      

  //TODO this is a good spot to throw an if statement and make software thresholds
  //if (Energy > .5)
  Front.Add(chan, Energy, low, high, time);
  multFront++;
}

void silicon::addBackEvent(unsigned short chan, unsigned short high, 
                           unsigned short low, unsigned short time)
{
  //Use calibration to get Energy and fill elist class in Telescope
  float Energy = BackEcal->getEnergy(0, chan, high);
  float time = BackTimecal->getTime(0, chan, time);

  //good spot to fill in all of the summary and chan spectrums
  Histo->sumWWBackE_R->Fill(chan, high);
  Histo->sumWWBackTime_R->Fill(chan, time);
  Histo->sumWWBackE_cal->Fill(chan, Energy);
  Histo->sumWWBackTime_cal->Fill(time);

  Histo->WWBackE_R[chan]->Fill(high);
  Histo->WWBackElow_R[chan]->Fill(low);
  Histo->WWBackTime_R[chan]->Fill(time);
  Histo->WWBackE_cal[chan]->Fill(Energy);      

  //TODO this is a good spot to throw an if statement and make software thresholds
  //if (Energy > .5)
  Back.Add(chan, Energy, low, high, time);
  multBack++;
}

void silicon::addDeltaEvent(unsigned short chan, unsigned short high, 
                            unsigned short low, unsigned short time)
{
  //Use calibration to get Energy and fill elist class in Telescope
  float Energy = DeltaEcal->getEnergy(0, chan, high);
  float time = DeltaTimecal->getTime(0, chan, time);

  //good spot to fill in all of the summary and chan spectrums
  Histo->sumWWDeltaE_R->Fill(chan, high);
  Histo->sumWWDeltaTime_R->Fill(chan, time);
  Histo->sumWWDeltaE_cal->Fill(chan, Energy);
  Histo->sumWWDeltaTime_cal->Fill(chan, time);

  Histo->WWDeltaE_R[chan]->Fill(high);
  Histo->WWDeltaElow_R[chan]->Fill(low);
  Histo->WWDeltaTime_R[chan]->Fill(time);
  Histo->WWDeltaE_cal[chan]->Fill(Energy);      

  //TODO this is a good spot to throw an if statement and make software thresholds
  //if (Energy > .5)
  Delta.Add(chan, Energy, low, high, time);
  multDelta++;
}

void gobbi::SiNeigbours()
{
  //When a charged particle moves through Si, there is cross talk between neighbouring
  //strips. Generally the signal is proportional to the total signal in the Si.
  Front.Neighbours(id);
  Back.Neighbours(id);
  Delta.Neighbours(id);
}




//*********************************************************
  //????????
void silicon::Reduce()
{
  multFront = Front.Reduce("F");
  multBack = Back.Reduce("B");
}

//***********************************************************
// subroutine to identify a single particle from strip data
int silicon::simpleFront()
{
  int dstrip = abs(Front.Order[0].strip - Delta.Order[0].strip) ;
  if (dstrip < -1 && dstrip > 3) 
  {
    Nsolution = 0;
    return 0;
  }

  if (fabs(Front.Order[0].energy - Back.Order[0].energy) > 2.) 
  {
    Nsolution = 0;
    return 0;
  }

  float timediff = Front.Order[0].time - Delta.Order[0].time;
  //cout << "Front.Order[0].time " << Front.Order[0].time << " - Delta.Order[0].time " << Delta.Order[0].time << " = " << timediff << endl;
  //if ( timediff < -500. || timediff > 100) 
  //{
  //  Nsolution = 0;
  //  return 0;
  //}



  Solution[0].energy = Front.Order[0].energy;
  Solution[0].energyR = Front.Order[0].energyR;
  Solution[0].benergy = Back.Order[0].energy;
  Solution[0].benergyR = Back.Order[0].energyR;
  Solution[0].denergy = Delta.Order[0].energy;
  Solution[0].denergyR = Delta.Order[0].energyR;
  Solution[0].ifront = Front.Order[0].strip;
  Solution[0].iback = Back.Order[0].strip;
  Solution[0].ide = Delta.Order[0].strip;
  Solution[0].itele = id; 
  Solution[0].timediff = timediff;
  //Solution[0].Nbefore = Front.Order[i].Nbefore;
  //Solution[0].Norder = Front.Order[i].Norder;

  //cout << "ifront " << Solution[0].ifront << "  strip " << Front.Order[0].strip << endl;

  Nsolution = 1;
  return 1;
}

//*************************************
//finds particle identification - checks to see if particle is inside of z - bananas  
int silicon::getPID()
{
  int pidmulti = 0;

  for (int isol=0; isol<Nsolution; isol++)
  {

    Solution[isol].ipid = 0;


    float energy = Solution[isol].energy;
    float denergy = Solution[isol].denergy*cos(Solution[isol].theta);

    bool FoundPid = Pid->getPID(energy, denergy);

    //no particle id is found
    if (!FoundPid) continue;
    else pidmulti++;

    Solution[isol].ipid = 1; //this can be adapted to be different values later
    Solution[isol].iZ = Pid->Z;
    Solution[isol].iA = Pid->A;
    Solution[isol].mass = Pid->mass*m0; //we want mass in energy units not AMU
  }
  return pidmulti;
}

int silicon::calcEloss()
{
  for (int isol=0; isol<Nsolution; isol++)
  {
    //need PID to calculate energy loss
    if (!Solution[isol].ipid)
    {
      Solution[isol].Ekin = 0;
      return 0;
    }

    //kinetics calc, add Delta and energy for total energy
    float sumEnergy = Solution[isol].denergy + Solution[isol].energy;
    float pc_before = sqrt(pow(sumEnergy+Solution[isol].mass,2) - pow(Solution[isol].mass,2));
    float velocity_before = pc_before/(sumEnergy+Solution[isol].mass);

    float thick = TargetThickness/2/cos(Solution[isol].theta);

    float ein = losses->getEin(sumEnergy,thick,Solution[isol].iZ,Solution[isol].mass/m0);

    //out << "loss correction " << ein - sumEnergy << endl;

    Solution[isol].Ekin = ein;
    //calc momentum vector, energyTot, and velocity
    Solution[isol].getMomentum();


    //protons can punch through at high energies
    if (Solution[isol].iA == 1 && Solution[isol].iZ == 1)
    {
      if (Solution[isol].Ekin > 15.5)
      {
        Solution[isol].iA = 0;
        Solution[isol].iZ = 0;
        Solution[isol].Ekin = 0;
        return 0;
      }
    }

    //deuterons can punch through
    if (Solution[isol].iA == 2 && Solution[isol].iZ == 1)
    {
      if (Solution[isol].Ekin > 20.5)
      {
        Solution[isol].iA = 0;
        Solution[isol].iZ = 0;
        Solution[isol].Ekin = 0;
        return 0;
      }
    }
    //Tritons can punch through
    if (Solution[isol].iA == 3 && Solution[isol].iZ == 1)
    {
      if (Solution[isol].Ekin > 24)
      {
        Solution[isol].iA = 0;
        Solution[isol].iZ = 0;
        Solution[isol].Ekin = 0;
        return 0;
      }
    }


  }




  return 1;
}




//****************************************************
//recursive subroutine  used for multihit subroutine
void silicon::loop(int depth)
{
  //cout << "depth=" << depth << " " << zline[0].n<< endl;
  if (depth == NestDim) //depth starts at 0
  {
    int dstrip = 0;
    float de = 0.;
    for (int i=0;i<NestDim;i++)
    {
      dstrip += abs(Delta.Order[NestArray[i]].strip
            -Front.Order[i].strip);
      de += abs(Back.Order[NestArray[i]].energy
            -Front.Order[i].energy);
    }

    //cout << " 1 " << zline[0].n << endl;

    if (dstrip < dstripMin)
    {
      dstripMin = dstrip;
      for (int i=0;i<NestDim;i++) {arrayD[i] = NestArray[i];}
    }
    //cout << " 2 " << zline[0].n << endl;

    if (de < deMin)
    {
      deMin = de;
      for (int i=0;i<NestDim;i++) {arrayB[i] = NestArray[i];}
    }
    //cout << "leave" << " " << zline[0].n << endl;
    return;
  }

  //cout << "recurse " << zline[0].n << endl;
  for (int i=0;i<NestDim;i++)
  {
    NestArray[depth] = i;
    int leave = 0;
    for (int j=0;j<depth;j++)
    {
      if (NestArray[j] == i)
      {
        leave =1;
        break; 
      }
    }
    if (leave) continue;
    loop(depth+1);
  }
}

//***************************************************
//extracts multiple particle from strip data 
int silicon::multiHit()
{
  int Ntries = min(Front.Nstore,Back.Nstore);
  Ntries = min(Ntries,Delta.Nstore);

  if (Ntries > 4) Ntries =4;
  Nsolution = 0;
  if (Ntries <= 0) return 0;
  
  for (NestDim = Ntries;NestDim>0;NestDim--)
  {
    dstripMin = 1000;
    deMin = 10000.;
    
    //look for best solution
    loop(0);
    
    //check to see if best possible solution is reasonable
    int leave = 0;
    for (int i=0;i<NestDim;i++)
    {
      if (abs(Delta.Order[arrayD[i]].strip - Front.Order[i].strip) > 2) 
      {
        leave = 1;
        break;
      }
      if (fabs(Back.Order[arrayB[i]].energy - Front.Order[i].energy) > 2.) 
      {
        leave = 1;
        break;
      }
      float timediff = Front.Order[i].time - Delta.Order[arrayD[i]].time;
      //if ( timediff < -500. || timediff > 100) 
      //{
      //  Nsolution = 0;
      //  return 0;
      //}
    }
      
    if (leave) continue;
    // now load solution
    for (int i=0;i<NestDim;i++)
    {
      float timediff = Front.Order[i].time - Delta.Order[arrayD[i]].time;
      Solution[i].energy = Front.Order[i].energy;
      Solution[i].energyR = Front.Order[i].energyR;
      Solution[i].denergy = Delta.Order[arrayD[i]].energy;
      Solution[i].ifront = Front.Order[i].strip;
      Solution[i].iback = Back.Order[arrayB[i]].strip;
      Solution[i].ide = Delta.Order[arrayD[i]].strip;
      Solution[i].itele = id;
      Solution[i].timediff = timediff;
      //Solution[i].Nbefore = Front.Order[i].Nbefore;
      //Solution[i].Norder = Front.Order[i].Norder;
    }

    Nsolution = NestDim;
    
    break;
  }
  return Nsolution;
}



//*******************************************************************************
  //calculates the x-y position and angles in the array in cm
void silicon::position(int isol)
{
  float Xpos,Ypos;

  if (id == 0) 
  {
    Xpos = Xcenter + 
          (((double)Solution[isol].iback+Ran->Rndm())/32.-0.5)*SiWidth;
    Ypos = Ycenter +
          (((double)Solution[isol].ifront+Ran->Rndm())/32.-0.5)*SiWidth;
  }
  else if (id == 1)
  {
    Xpos = Xcenter + 
          (((double)Solution[isol].ifront+Ran->Rndm())/32.-0.5)*SiWidth;
    Ypos = Ycenter +
          (0.5-((double)Solution[isol].iback+Ran->Rndm())/32.)*SiWidth;
  }
  else if (id == 2)
  {
    Xpos = Xcenter + 
          (0.5-((double)Solution[isol].iback+Ran->Rndm())/32.)*SiWidth;
    Ypos = Ycenter +
          (0.5-((double)Solution[isol].ifront+Ran->Rndm())/32.)*SiWidth;
  }
  else if (id == 3)
  {
    Xpos = Xcenter + 
          (0.5-((double)Solution[isol].ifront+Ran->Rndm())/32.)*SiWidth;
    Ypos = Ycenter +
          (((double)Solution[isol].iback+Ran->Rndm())/32.-0.5)*SiWidth;
  }

  //  Xpos += .3;

  Solution[isol].Xpos = Xpos;
  Solution[isol].Ypos = Ypos;
  float theta = Solution[isol].angle();
}
//***********************************************************************

//*******************************************************************************
  //calculates the x-y position in the array in cm
void silicon::positionC(int isol)
{
  float Xpos,Ypos;

  if (id == 0) 
  {
    Xpos = Xcenter +
         (((double)Solution[isol].iback+.5)/32.-0.5)*SiWidth;
    Ypos = Ycenter +
         (((double)Solution[isol].ifront+.5)/32.-0.5)*SiWidth;
  }
  else if (id == 1)
  {
    Xpos = Xcenter + 
          (((double)Solution[isol].ifront+.5)/32.-0.5)*SiWidth;
    Ypos = Ycenter +
          (0.5-((double)Solution[isol].iback+.5)/32.)*SiWidth;
  }
  else if (id == 2)
  {
    Xpos = Xcenter +
          (0.5-((double)Solution[isol].iback+.5)/32.)*SiWidth;
    Ypos = Ycenter +
          (0.5-((double)Solution[isol].ifront+.5)/32.)*SiWidth;
  }
  else if (id == 3)
  {
    Xpos = Xcenter +
          (0.5-((double)Solution[isol].ifront+.5)/32.)*SiWidth;
    Ypos = Ycenter +
          (((double)Solution[isol].iback+.5)/32.-0.5)*SiWidth;
  }
  Solution[isol].Xpos = Xpos;
  Solution[isol].Ypos = Ypos;
  float theta = Solution[isol].angle();
}
//***********************************************************************


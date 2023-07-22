//Nicolas Dronchi 2023_07_13
//class dealing with a single dE-E-E [Si-Si-CsI] telescope

#include "telescope.h"

using namespace std;

//**********************************************************
  //constructor
telescope::telescope()
{
  SiWidth = 6.45;
  WWidth = 4.95; //cm
  //switch which loss file is used depending on the target.
  losses = new CLosses(6,"_CD2.loss");

  Ran = new TRandom();
}

//***********************************************************
//destructor
telescope::~telescope()
{
  delete losses;
  delete Ran;
  delete Pid;
}

//inialization
void telescope::init(int id0)
{
  id = id0;
  //-ND checked 7/13/2023 these distances are correct for a 2cmx2cm Gobbi hole
  //    ______ Front view of Gobbi
  //    |    |____                  
  //    | 4  |    |              
  //   _|____| 1  |    __
  //--|--->|_|____|-->|5 | WW behind
  //  | 3  |    |     |__|
  //  |____| 2  |
  //       |____|
  float const XcenterA[5] = {4.624,2.631,-4.624,-2.631, 0};
  float const YcenterA[5] = {2.631,-4.624,-2.631,4.624, 0};
  Xcenter = XcenterA[id];
  Ycenter = YcenterA[id];

  ostringstream outstring;  
  outstring << "pid_quad" << id+1;
  SiSiPid = new pid(outstring.str());
  outstring << "pid_quad" << id+1 << "_CsI";
  SiCsIPid = new pid(outstring.str());

}

void telescope::SetTargetDistance(double dist, float thick)
{
  for (int i=0;i<10;i++){ Solution[i].SetTargetDistance(dist); }
  TargetThickness = thick;
}


//********************************************************
void telescope::reset()
{
  multFront = 0;
  multBack = 0;
  multDelta = 0;
  multCsI = 0;

  Front.reset();
  Back.reset();
  Delta.reset();
  CsI.reset();

  //possible to loop to Nsolution (but safer to just loop over all possible solution)
  for (int i=0; i<10; i++){ Solution[i].reset();}

  Nsolution = 0;
}

//*********************************************************
  //????????
void telescope::Reduce()
{
  multFront = Front.Reduce("F");
  multBack = Back.Reduce("B");
}













//***********************************************************
// subroutine to identify a single particle from dE-E [Si-Si] data
int telescope::SimpledEE()
{
  int dstrip = abs(Front.Order[0].strip - Delta.Order[0].strip);
  if (dstrip > 3)
  {
    Nsolution = 0;
    return 0;
  }

  //this won't work until your calibrations are good.
  if (fabs(Front.Order[0].energy - Back.Order[0].energy) > 2.) 
  {
    Nsolution = 0;
    return 0;
  }

  float timediff = Front.Order[0].time - Delta.Order[0].time;

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
  Solution[0].isSiCsI = false;

  Nsolution = 1;
  return 1;
}
// same as above but for E-CsI events
int telescope::SimpleECsI()
{
  //this needs some logic to double check, maybe something like checking if the CsI
  //is behind the x-y potions of the E?
  //if (Front.Order[0].strip <= 15 && Back.Order[0].strip <= 15 && CsI.Order[0].strip !=1)
  //{
  //  Nsolution = 0;
  //  return 0;
  //}
  //else if (Front.Order[0].strip <= 15 && Back.Order[0].strip > 15 && CsI.Order[0].strip !=2)
  //{
  //  Nsolution = 0;
  //  return 0;
  //}
  //else if (Front.Order[0].strip > 15 && Back.Order[0].strip <= 15 && CsI.Order[0].strip !=3)
  //{
  //  Nsolution = 0;
  //  return 0;
  //}
  //else if (Front.Order[0].strip > 15 && Back.Order[0].strip > 15 && CsI.Order[0].strip !=4)
  //{
  //  Nsolution = 0;
  //  return 0;
  //}

  //this won't work until your calibrations are good. but it should be turned on.
  if (fabs(Front.Order[0].energy - Back.Order[0].energy) > 2.) 
  {
    Nsolution = 0;
    return 0;
  }

  float timediff = CsI.Order[0].time - Front.Order[0].time;

  Solution[0].energy = CsI.Order[0].energy;
  Solution[0].energyR = CsI.Order[0].energyR;
  Solution[0].benergy = Back.Order[0].energy;
  Solution[0].benergyR = Back.Order[0].energyR;
  Solution[0].denergy = Front.Order[0].energy;
  Solution[0].denergyR = Front.Order[0].energyR;
  Solution[0].ifront = CsI.Order[0].strip; //don't know if i want to do this
  Solution[0].iback = Back.Order[0].strip;
  Solution[0].ide = Front.Order[0].strip;
  Solution[0].itele = id; 
  Solution[0].timediff = timediff;
  Solution[0].isSiCsI = true;
  Solution[0].itele = id; 

  Nsolution = 1;
  return 1;
}

//****************************************************
//recursive subroutine  used for multihit subroutine
//this is a complicated subroutine and I forget how it works every time -ND
void telescope::loop(int depth)
{
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

    if (dstrip < dstripMin)
    {
      dstripMin = dstrip;
      for (int i=0;i<NestDim;i++) {arrayD[i] = NestArray[i];}
    }

    if (de < deMin)
    {
      deMin = de;
      for (int i=0;i<NestDim;i++) {arrayB[i] = NestArray[i];}
    }
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
//recursive subroutine  used for multihit subroutine
void ringCounter::loop(int depth)
{
  if (depth == NestDim )
  {
    // do stuff here
    int dstrip = 0;
    float de = 0.;
    for (int i=0;i<NestDim;i++)
    {
      pie_energy = Pie.Order[NestArray[i]].energy;
      ring_energy = Ring.Order[i].energy;
      de += abs(pie_energy-ring_energy);
    }

    if (dstrip < dstripMin)
    {
      dstripMin = dstrip;
      for (int i=0;i<NestDim;i++) {arrayD[i] = NestArray[i];}
    }

    if (de < deMin)
    {
      deMin = de;
      for (int i=0;i<NestDim;i++) {arrayB[i] = NestArray[i];}
    }

    return;
  }


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
int telescope::multiHitdEE()
{
  int Ntries = min(Front.Nstore,Back.Nstore);
  Ntries = min(Ntries,Delta.Nstore);

  if (Ntries > 4)
    Ntries = 4;
  if (Ntries <= 0)
    return 0;

  Nsolution = 0;
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
      Solution[i].isSiCsI = false;
    }

    Nsolution = NestDim;
    
    break;
  }
  return Nsolution;
}


//TODO needs some work!
//***************************************************
//modification of multiHitdEE() to match E-CsI events
int telescope::multiHitECsI()
{
  int Ntries = min(Front.Nstore,Back.Nstore);
  Ntries = min(Ntries,CsI.Nstore);

  if (Ntries > 4)
    Ntries = 4;
  if (Ntries <= 0)
    return 0;

  Nsolution = 0;
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
      if (fabs(Back.Order[arrayB[i]].energy - Front.Order[i].energy) > 2.) 
      {
        leave = 1;
        break;
      }

      //TODO
      //something like this again but use the correct arrayB
      //if (Front.Order[0].strip <= 15 && Back.Order[0].strip <= 15 && CsI.Order[0].strip !=1)
      //{
      //  leave = 1;
      //  break;
      //}
      //else if (Front.Order[0].strip <= 15 && Back.Order[0].strip > 15 && CsI.Order[0].strip !=2)
      //{
      //  leave = 1;
      //  break;
      //}
      //else if (Front.Order[0].strip > 15 && Back.Order[0].strip <= 15 && CsI.Order[0].strip !=3)
      //{
      //  leave = 1;
      //  break;
      //}
      //else if (Front.Order[0].strip > 15 && Back.Order[0].strip > 15 && CsI.Order[0].strip !=4)
      //{
      //  leave = 1;
      //  break;
      //}
    }
      
    if (leave) continue;
    // now load solution
    for (int i=0;i<NestDim;i++)
    {
      float timediff = CsI.Order[i].time - Front.Order[arrayD[i]].time;
      Solution[i].energy = CsI.Order[i].energy;
      Solution[i].energyR = CsI.Order[i].energyR;
      Solution[i].denergy = Front.Order[arrayD[i]].energy;
      Solution[i].ifront = CsI.Order[i].strip;
      Solution[i].iback = Back.Order[arrayB[i]].strip;
      Solution[i].ide = Front.Order[arrayD[i]].strip;
      Solution[i].itele = id;
      Solution[i].timediff = timediff;
      Solution[i].isSiCsI = true;
    }

    Nsolution = NestDim;
    
    break;
  }
  return Nsolution;
}
































//*************************************
//finds particle identification - checks to see if particle is inside of z - bananas  
int telescope::getPID()
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

int telescope::calcEloss()
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
  }

  return 1;
}






//*******************************************************************************
  //calculates the x-y position and angles in the array in cm
void telescope::position(int isol)
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
void telescope::positionC(int isol)
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


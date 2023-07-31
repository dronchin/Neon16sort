//Nicolas Dronchi 2023_07_13
//class dealing with a single dE-E-E [Si-Si-CsI] telescope
//       .-.      _______                             .  '  *   .  . '
//      {}``; |==|_______D                              .  * * -+-  
//      / ('        /|\                             .    * .    '  *
//  (  /  |        / | \                                * .  ' .  . 
//   \(_)_]]      /  |  \                            *   *  .   .
//                                                     '   *
#include "telescope.h"

using namespace std;

//constructor
telescope::telescope()
{
  //Verify these numbers
  SiWidth = 6.45;
  WWidth = 4.95; //cm
  //switch which loss file is used depending on the target.
  Targlosses = new CLosses(8,"_Be.loss");
  PbSnlosses = new CLosses(8,"_PbSn.loss");
  Silosses = new CLosses(8,"_Si.loss");

  Ran = new TRandom();

  //CsI calibrations for clusters from proton calibrations
  string name = "Cal/csi/deuteron.cal";
  calCsi_d = new calibrate(1,16,name,1,false);
  name = "Cal/csi/triton.cal";
  calCsi_t = new calibrate(1,16,name,1,false);
  //name = "Cal/csi/He3.cal";
  //calCsi_3He = new calibrate(1,16,name,1);
  name = "Cal/csi/alpha.cal";
  calCsi_Alpha = new calibrate(1,16,name,1,false);

}

//destructor
telescope::~telescope()
{
  delete Targlosses;
  delete PbSnlosses;
  delete Ran;
  delete Pid;
  for (int i=0;i<4;i++)  delete PidECsI[i];
  delete calCsi_d;
  delete calCsi_t;
  delete calCsi_Alpha;
}

//inialization
void telescope::init(int id0)
{
  id = id0;
  //-ND checked 7/13/2023 these distances are correct for a 2cmx2cm Gobbi hole
  //     ____  Front view of Gobbi
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
  Pid = new pid(outstring.str());

  for (int i=0;i<4;i++)
  {
    outstring << "pid_quad" << id+1 << "_CsI"<< i+1;
    PidECsI[i] = new pid(outstring.str());
  }

}

void telescope::SetTarget(double dist, float thick)
{
  for (int i=0;i<10;i++){ Solution[i].SetTargetDistance(dist); }
  TargetThickness = thick;
}

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

//????????
void telescope::Reduce()
{
  multFront = Front.Reduce("F");
  multBack = Back.Reduce("B");
}



//  .--.      .-'.      .--.      .--.      .--.      .--.      .`-.      .--.
//:::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\::::::::.\
//'      `--'      `.-'      `--'      `--'      `--'      `-.'      `--'      `
//Here are 5 subroutines that are called to figure out how to go from detector 
//data to solutions that track a single particle.
//  -testingHitE()
//  -SimpledEE()
//  -SimpleECsI()
//  -multiHitdEE() + loop()
//  -multiHitECsI() + loop()


// subroutine to identify a position map from alpha calibrations 
// it stores the answer in the last solution.
int telescope::testingHitE()
{
  Solution[9].energy = Front.Order[0].energy;
  Solution[9].energyR = Front.Order[0].energyR;
  Solution[9].benergy = Back.Order[0].energy;
  Solution[9].benergyR = Back.Order[0].energyR;
  Solution[9].denergy = -1;
  Solution[9].denergyR = -1;;
  Solution[9].ifront = Front.Order[0].strip;
  Solution[9].iback = Back.Order[0].strip;
  Solution[9].ide = -1;
  Solution[9].iCsI = -1;
  Solution[9].itele = id;
  Solution[9].timediff = -1000000.0;
  Solution[9].isSiCsI = false;

  Nsolution = 0; //don't determine there is any solutions.
  return 0;
}


//***********************************************************
// subroutine to identify a single particle from dE-E [Si-Si] data
int telescope::simpledEE()
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
  Solution[0].iCsI = -1;
  Solution[0].itele = id; 
  Solution[0].timediff = timediff;
  Solution[0].isSiCsI = false;

  Nsolution = 1;
  return 1;
}
// same as above but for E-CsI events
int telescope::simpleECsI()
{
  //here we check the CsI is behind the x-y potions of the E
  //if it doesn't match, it is removed so there is a chance it is a dE-E event
  if (Front.Order[0].strip <= 15 && Back.Order[0].strip <= 15 && 
      CsI.Order[0].strip !=0)
  {
    Nsolution = 0;
    CsI.Remove(0);
    return 0;
  }
  else if (Front.Order[0].strip <= 15 && Back.Order[0].strip > 15 && 
      CsI.Order[0].strip !=1)
  {
    Nsolution = 0;
    CsI.Remove(0);
    return 0;
  }
  else if (Front.Order[0].strip > 15 && Back.Order[0].strip > 15 && 
      CsI.Order[0].strip !=2)
  {
    Nsolution = 0;
    CsI.Remove(0);
    return 0;
  }
  else if (Front.Order[0].strip > 15 && Back.Order[0].strip <= 15 && 
      CsI.Order[0].strip !=3)
  {
    Nsolution = 0;
    CsI.Remove(0);
    return 0;
  }

  //this won't work until your calibrations are good. but it should be turned on.
  if (fabs(Front.Order[0].energy - Back.Order[0].energy) > 2.) 
  {
    Nsolution = 0;
    CsI.Remove(0);
    return 0;
  }

  float timediff = CsI.Order[0].time - Front.Order[0].time;

  Solution[0].energy = CsI.Order[0].energy;
  Solution[0].energyR = CsI.Order[0].energyR;
  Solution[0].benergy = Back.Order[0].energy;
  Solution[0].benergyR = Back.Order[0].energyR;
  Solution[0].denergy = Front.Order[0].energy;
  Solution[0].denergyR = Front.Order[0].energyR;

  Solution[0].ifront = Front.Order[0].strip;
  Solution[0].iback = Back.Order[0].strip;
  Solution[0].ide = -1;
  Solution[0].iCsI= CsI.Order[0].strip;
  Solution[0].itele = id; 
  Solution[0].timediff = timediff;
  Solution[0].isSiCsI = true;
  Solution[0].itele = id;

  Nsolution = 1;
  return 1;
}

//****************************************************
//recursive subroutine  used for multihit subroutine
//this is a complicated subroutine so i have a mega comment labled multihit_comment.h 
//stored in this directory. It will step you through a multihit Si-Si event
void telescope::loop(int depth)
{
  //only work in this section if we are at the max level of recursion
  if (depth == NestDim) 
  {
    //as an example for NestDim = 2
    //for first time through on loop(2) nestarray = {0,1} so we check
    //if highest energy delta matches with highest energy Front. Then 
    //check if second highest delta matches highest energy Front.

    int dstrip = 0;
    float de = 0.;
    for (int i=0;i<NestDim;i++)
    {
      //difference in strip number is how to match the dE and E events (closest wins)
      dstrip += abs(Delta.Order[NestArray[i]].strip - Front.Order[i].strip);

      //difference in Front and back energy is how to match FrontE and BackE
      de += abs(Back.Order[NestArray[i]].energy
            -Front.Order[i].energy);
    }

    //here if it is the lowest total difference in strip# or energy, it is saved in
    //arrayD (for dE matching) and arrayB (for matching front-back)
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

  //this section handles how deep we go into the recursion loop.
  //the key to this section if figuring out what NestArrays to check.
  //for NestDim=2, we want to check {0,1} and {1,0}
  for (int i=0;i<NestDim;i++)
  {
    NestArray[depth] = i;
    int leave = 0;
    //when matching we want to skip items already matched 
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
      Solution[Nsolution].energy = Front.Order[i].energy;
      Solution[Nsolution].energyR = Front.Order[i].energyR;
      Solution[Nsolution].denergy = Delta.Order[arrayD[i]].energy;
      Solution[Nsolution].ifront = Front.Order[i].strip;
      Solution[Nsolution].iback = Back.Order[arrayB[i]].strip;
      Solution[Nsolution].ide = Delta.Order[arrayD[i]].strip;
      Solution[Nsolution].iCsI = -1;
      Solution[Nsolution].itele = id;
      Solution[Nsolution].timediff = timediff;
      Solution[Nsolution].isSiCsI = false;
      Nsolution++;
    }

    
    break;
  }
  return Nsolution;
}









//***************************************************
//recursive subroutine  used for multihit specifically for E-CsI case
//it is actually simpler in the case we only need to match Front/Back. No need to match
//dE to this as well. The logic for matching to CsI is in multiHitECsI().
//this is a complicated subroutine so i have a mega comment labled multihit_comment.h 
//stored in this directory. It will step you through a multihit Si-Si event.

//Actually i don't think we need a loop without just filling in the arrayD.
//in the original version of loop(), it will potentiall access Delta.Order[i]
//but these exist (we are not indexing out of range). It will just fill in 
//arrayD with a useless answer. -ND
/*
void telescope::loop_v2(int depth)
{
  if (depth == NestDim )
  {
    float de = 0.;
    for (int i=0;i<NestDim;i++)
    {
      //difference in Front and back energy is how to match FrontE and BackE
      de += abs(Back.Order[NestArray[i]].energy
            -Front.Order[i].energy);
    }

    if (de < deMin)
    {
      deMin = de;
      for (int i=0;i<NestDim;i++) {arrayB[i] = NestArray[i];}
    }

    return;
  }

  //this section handles how deep we go into the recursion loop.
  //the key to this section if figuring out what NestArrays to check.
  //for NestDim=2, we want to check {0,1} and {1,0}
  for (int i=0;i<NestDim;i++)
  {
    NestArray[depth] = i;
    int leave = 0;
    //when matching we want to skip items already matched 
    for (int j=0;j<depth;j++)
    {
      if (NestArray[j] == i)
      {
        leave =1;
        break; 
      }
    }
    if (leave) continue;
    loop_v2(depth+1);
  }
}
*/


//TODO this was all new code and needs to be checked over thoroughly
//***************************************************
//modification of multiHitdEE() to match E-CsI events
int telescope::multiHitECsI()
{
  int Ntries = min(Front.Nstore,Back.Nstore);
  //Ntries = min(Ntries,CsI.Nstore);

  if (Ntries > 4)
    Ntries = 4;
  if (Ntries <= 0)
    return 0;

  NSisolution = 0;
  for (NestDim = Ntries;NestDim>0;NestDim--)
  {
    dstripMin = 1000;
    deMin = 10000.;
    
    //look for best Front/Back matching
    //solutions are stored in arrayB
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
    }
    if (leave) continue;
    NSisolution = NestDim;
  }
  //save some time and just exit if there are no Si Front/Back solutions
  if (NSisolution == 0) return 0;

  //now assign each of these solutions a Csi detector location 
  int mult[4]={0};  //array for multipility of Si solution for each Csi
  int sil[4][10];   //contains a lits of solutions for each Csi

  //look at all the Front/Back solutions and see how many are on each CsI
  for (int i=0;i<NSisolution;i++)
  {
    int ifront = Front.Order[i].strip;
    int iback = Back.Order[arrayB[i]].strip;
    for (int icsi=0;icsi<4;icsi++)
    {
      if (ifront >= FrontLow[icsi] &&
          ifront <= FrontHigh[icsi] &&
          iback  >= BackLow[icsi]  &&
          iback  <= BackHigh[icsi])
      {
        sil[icsi][mult[icsi]] = i;
        mult[icsi]++;
        break;
      }
    }
  }

  //make array of detect csi energies
  float energy[4]={-1.};
  float energyR[4]={-1.};
  short order[4]={-1};

  //store the CsI raw energy info in an array that corresponds to the position it is in
  for (int i=0;i<CsI.Nstore;i++)
  {
    energy[CsI.Order[i].strip] = CsI.Order[i].energy;
    order[CsI.Order[i].strip] = i;
  }

  //loop over csi location
  for (int icsi = 0;icsi<4;icsi++)
  {
    //no solution for this location, ignore
    if (mult[icsi] == 0) continue;
    //more than one si solution for a single Csi location ignore
    else if (mult[icsi] > 1) continue;
    // CsI energy < 0 should not happen
    //but just in case ignore
    else if(energy[icsi] <= 0.) continue;
    
    else
    {
      int ii = sil[icsi][0];
      Solution[Nsolution].energy = energy[icsi];
      Solution[Nsolution].energyR = CsI.Order[order[icsi]].energyR;
      Solution[Nsolution].denergy = Front.Order[ii].energy;
      Solution[Nsolution].denergyR = Front.Order[ii].energyR;
      Solution[Nsolution].benergy = Back.Order[arrayB[ii]].energy;
      Solution[Nsolution].benergyR = Back.Order[arrayB[ii]].energy;

      Solution[Nsolution].ifront = Front.Order[ii].strip;
      Solution[Nsolution].iback = Back.Order[arrayB[ii]].strip;
      Solution[Nsolution].ide = -1;
      Solution[Nsolution].iCsI = icsi;
      Solution[Nsolution].itele = id;
      Solution[Nsolution].isSiCsI = true;
      float timediff = CsI.Order[order[icsi]].time - Front.Order[ii].time;
      Solution[Nsolution].timediff = timediff;
      Nsolution++;
    }
  }
  
  return Nsolution;
}
/*
//ERROR PidECsI is only an array of 4 in here. By principle, a single telescope
//should not load in all the calibration/PID info. Just the 4 it needs. -ND

//also everything after this is for calculating PID, do we need to do that for a solution?
//because if we calculate PID here, then we need to also calculate it in the simpleECsI.
//This method should just fill in the solution matched. I suggest just filling in the
//solutions here and leaving the energy part empty. Then when we do PID later, we
//can figure out the energy there. -ND
//I would prefer if the energy calibrations were stored in Gobbi as a 4x4 array for each pid
//somewhere the CsI energy histograms need to be filled.

      bool stat = PidECsI[icsi]->getPID(energyR[icsi],Solution[ii].denergy);
      if(!stat)
      {
        Solution[ii].energy =0.;
        Solution[ii].iZ =0;
        continue;
      }
      int Z = Pid[icsi]->Z;
      int A = Pid[icsi]->A;



      int ifront = Solution[ii].ifront;
      int iback = Solution[ii].iback;

      
        //** theta and phi calculated in follow function
      position[ii];


      if(Z >0 && A>0)
      {
        Solution[ii].penergy = energy[icsi];

        energy[icsi] = light2energy(Z,A,CsIhit,energy[icsi]);

        float sumenergy = energy[icsi] + Solution[ii].denergy;

        float thick = 193./2./cos(Solution[ii].theta);

        float Ein;
        Ein  = losses->getEin(sumenergy,thick,Z,A);
        float Ekin = Ein;




        Solution[ii].energy = energy[icsi];
        Solution[ii].energyR = energyR[icsi];
        Solution[ii].icsi = icsi;

          Solution[ii].energyCsI = Csi.Order[order[icsi]].energyCsI;
          Solution[ii].timeCsI = Csi.Order[order[icsi]].time;
          Solution[ii].iZ = Z;
        Solution[ii].iA = A;
        Solution[ii].mass = getMass(Z,A);
        Solution[ii].Ekin = Ekin;
        Solution[ii].itele = id;

        //need to have use my flag to identify what type of solution it is -ND
        Solution[ii].isSiCsI = true;

      }
    }
  }
 */ 


//code from ND solution for mulithitECsI() could be how you find ECsI and dEE in same go
/* 
  Nsolution = 0;
  while (CsI.Nstore > 0)
  //for (int icsi = 0; icsi<CsI.Nstore; icsi++)
  {
    //will need to check there is one solution for each CsI
    //solutions matched will be stored by index in isimatched
    int NSimatches = 0;
    int isimatched = -1;
    for (int isi = 0; isi<NSisolution; isi++)
    {
      //if the front back solution isn't in the correct CsI then it is not a solution
      //each rotation 
      // Back 0->31  
      // [3]|[2] Front 31
      // -------       ^
      // [0]|[1]       0
      if (Front.Order[isi].strip <= 15 &&
          Back.Order[arrayB[isi]].strip <= 15 && 
          CsI.Order[0].strip ==0)
      {
        NSimatches++;
        isimatched = isi;
      }
      if (Front.Order[isi].strip <= 15 &&
          Back.Order[arrayB[isi]].strip > 15 && 
          CsI.Order[0].strip ==1)
      {
        NSimatches++;
        isimatched = isi;
      }
      if (Front.Order[isi].strip > 15 &&
          Back.Order[arrayB[isi]].strip > 15 && 
          CsI.Order[0].strip ==2)
      {
        NSimatches++;
        isimatched = isi;
      }
      if (Front.Order[isi].strip > 15 &&
          Back.Order[arrayB[isi]].strip <= 15 && 
          CsI.Order[0].strip ==3)
      {
        NSimatches++;
        isimatched = isi;
      }
    }
    //if two FrontBack pairs hit the same CsI quadrant it will be impossible to
    //tell which one is which. Get rid of these events
    //also need to keep moving if none of the Front/Back solutions match the correct Si
    if (NSimatches != 1) 
    {
      CsI.Remove(0);
      continue;
    }
    // now load solution
    Solution[Nsolution].energy = CsI.Order[icsi].energy;
    Solution[Nsolution].energyR = CsI.Order[icsi].energyR;
    Solution[Nsolution].denergy = Front.Order[isimatched].energy;

    Solution[Nsolution].ifront = Front.Order[isimatched].strip;
    Solution[Nsolution].iback = Back.Order[arrayB[isimatched]].strip;
    Solution[Nsolution].ide = -1;
    Solution[Nsolution].iCsI = CsI.Order[icsi].strip;

    Solution[Nsolution].itele = id;
    Solution[Nsolution].timediff = CsI.Order[icsi].time - Front.Order[isimatched].time;
    Solution[Nsolution].isSiCsI = true;

    //when we have a good match, remove these from the elist so that it is possible
    //to find extra solutions after with simpledEE() or multiHitdEE()
    CsI.Remove(0);
    Front.Remove(isimatched);
    Back.Remove(arrayB[isimatched]);

    Nsolution++;
  }
*/










//*************************************
//finds particle identification - checks to see if particle is inside of z - bananas  
int telescope::getPID()
{
  int pidmulti = 0;
  for (int isol=0; isol<Nsolution; isol++)
  {
    Solution[isol].ipid = 0;
    bool isSiCsI = Solution[isol].isSiCsI;

    float energy = Solution[isol].energy;
    float energyR = Solution[isol].energyR;
    float denergy = Solution[isol].denergy*cos(Solution[isol].theta);

    bool FoundPid = false;
    if (isSiCsI)
    {
      //use raw energy for CsI PID
      FoundPid = PidECsI[Solution[isol].iCsI]->getPID(energyR, denergy);
      Pid->Z = PidECsI[Solution[isol].iCsI]->Z;
      Pid->A = PidECsI[Solution[isol].iCsI]->A;
      Pid->mass = PidECsI[Solution[isol].iCsI]->mass;
    }
    else
    {
      FoundPid = Pid->getPID(energy, denergy);
    }

    //no particle id is found
    if (!FoundPid) continue;
    else pidmulti++;

    Solution[isol].ipid = 1; //this can be adapted to be different values later
    Solution[isol].iZ = Pid->Z;
    Solution[isol].iA = Pid->A;
    Solution[isol].mass = Pid->mass*m0; //we want mass in energy units not AMU
  
    //take proton equivalent energies to light equivalent
    if (isSiCsI)
    {
      int csid = Solution[isol].iCsI + 4*id;
      Solution[isol].energy = light2energy(Pid->Z, Pid->A, csid, Solution[isol].energy);
    }
  }
  return pidmulti;
}
//********************************************************************
//TODO add in PbSn if statement for id = 4
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

    //Double check this value. seems kind of small.
    float PbSnThick = 5.06/cos(Solution[isol].theta); // mg/cm^2
    if (Solution[isol].itele == 4) 
    {
      sumEnergy = PbSnlosses->getEin(sumEnergy,PbSnThick,Solution[isol].iZ,Solution[isol].mass/m0);
    }  
    
    //need to account for energy loss in dE
    float dEthick = 15.0865/cos(Solution[isol].theta); // Si 65 mm = 15.0865 mg/cm^2
    if (Solution[isol].isSiCsI)
    {
      sumEnergy = Silosses->getEin(sumEnergy,dEthick,Solution[isol].iZ,Solution[isol].mass/m0);
    }
    
    float thick = TargetThickness/2/cos(Solution[isol].theta);
   
    float ein = Targlosses->getEin(sumEnergy,thick,Solution[isol].iZ,Solution[isol].mass/m0);

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

  //WW pins go up in telescope, should be the same mapping as Gobbi telescope 1
  //The following is true if (0,0) is in the upper left hand
  // _ _ _ _
  //|_|_|_|_| 4B31
  //|_|_|_|_|  ^
  //|_|_|_|_|  |
  //|_|_|_|_| 4B16
  //4F16 -> 5F31
  else if (id == 4)
  {
    Xpos = Xcenter + 
          (((double)Solution[isol].ifront+Ran->Rndm())/16.-0.5)*SiWidth;
    Ypos = Ycenter +
          (((double)Solution[isol].iback+Ran->Rndm())/16.-0.5)*SiWidth;
  }

  //  Xpos += .3;

  Solution[isol].Xpos = Xpos;
  Solution[isol].Ypos = Ypos;
  float theta = Solution[isol].angle();
}
//***********************************************************************

/* only need positionC if you don't want to randomly distribute event in a strip
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
*/

//***********************************************************
// load front and back strip delimeters for each CsI crystal.
void telescope::load(int F0low, int F1low,int F2low, int F3low,
                 int F0hi,  int F1hi, int F2hi,  int F3hi,
                 int B0low, int B1low,int B2low, int B3low,
                 int B0hi,  int B1hi, int B2hi,  int B3hi)
{
  FrontLow[0] = F0low;
  FrontLow[1] = F1low;
  FrontLow[2] = F2low;
  FrontLow[3] = F3low;

  FrontHigh[0] = F0hi;
  FrontHigh[1] = F1hi;
  FrontHigh[2] = F2hi;
  FrontHigh[3] = F3hi;

  BackLow[0] = B0low;
  BackLow[1] = B1low;
  BackLow[2] = B2low;
  BackLow[3] = B3low;

  BackHigh[0] = B0hi;
  BackHigh[1] = B1hi;
  BackHigh[2] = B2hi;
  BackHigh[3] = B3hi;

}

//***************************************************
// converstion of equilivant proton energy to energy for a given isotope
//i.e. Z and A dependence of CsI light output
float telescope::light2energy(int Z, int A, int CsIhit, float energy)
{
  
  if(Z ==1)
  { 
    if(A ==2)
      energy = calCsI_d->getEnergy(0,CsIhit,energy);
    if(A ==3)
      energy = calCsI_t->getEnergy(0,CsIhit,energy);
  }
  else if(Z == 2)
  {
    if(A ==3)
      energy = calCsI_Alpha->getEnergy(0,CsIhit,energy);
    else if(A ==4)
      energy = calCsi_Alpha->getEnergy(0,CsIhit,energy);
    else 
    {
      //cout << "found no calib for " << Z << " " << A << endl; 
      return -1.;
    }
  }
  else 
  {
    cout << "found no calib for Z= " << Z << " A= " << A << " id =  " << id << " Csi = " << CsIhit << endl; 
    abort();
  }

  return energy;
}
//***********************************************************************



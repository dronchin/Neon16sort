#include "tele.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

TRandom CTele::ran;

/**
 * Constructor
\param r_center0 is vector pointing to center of Si detector (cm)
\param r_front0 is unit vector pointing along front strips
\param r_back0 is unit vector pointing along back strips
\param xActive0 is in-plane active dimension of Si in cm
\param yActive0 is the out-of-plane dimension
 */

CTele::CTele(float* r_center0, float *r_front0, float *r_back0, float xActive0, 
float yActive0)
{
  init(r_center0,r_front0,r_back0,xActive0,yActive0);
}
//************************************************************
/**
 * initialize parameters of a HiRA detector
\param r_center0 is vector pointing to center of Si detector (cm)
\param r_front0 is unit vector pointing along front strips
\param r_back0 is unit vector pointing along back strips
\param xActive0 is in-plane active dimension of Si in cm
\param yActive0 is the out-of-plane dimension
 */
void CTele::init(float* r_center0, float* r_front0, float *r_back0,
  float xActive0, float yActive0)

{

  for (int i=0;i<3;i++)
    {
      r_center[i] = r_center0[i];
      r_front[i] = r_front0[i];
      r_back[i] = r_back0[i];
    }
  xActive = xActive0;
  yActive = yActive0; 
}

//************************************************************************
  /**
   * logical indicating that the HiRA detector was hit
\param theta is zenith angle of particle in radians
\param phi is azimuth angle of particle in rdians
\param xtarget in-plane location of interaction in target from nomimal center (cm)
\param ytarget out-of-plane location (cm)
   */
int CTele::hit(float theta, float phi, float xtarget, float ytarget)
{

  sle Sle(3);
 
  Sle.M[0][0] = sin(theta)*cos(phi);
  Sle.M[0][1] = -r_front[0];
  Sle.M[0][2] = -r_back[0];

  Sle.M[1][0] = sin(theta)*sin(phi);
  Sle.M[1][1] = -r_front[1];
  Sle.M[1][2] = -r_back[1];

  Sle.M[2][0] = cos(theta);
  Sle.M[2][1] = -r_front[2];
  Sle.M[2][2] = -r_back[2];

  Sle.Y[0] = r_center[0] - xtarget;
  Sle.Y[1] = r_center[1] - ytarget;
  Sle.Y[2] = r_center[2];

  Sle.solve();

  float dist_front = Sle.Y[1];
  float dist_back = Sle.Y[2];

  int hitit = 0;
  if (fabs(dist_front) < xActive/2. && fabs(dist_back) < yActive/2.) 
    hitit = 1;

  if (hitit == 0) return hitit;


  //find strip hit
  ixStrip = (int)((dist_front + xActive/2.)/xActive*32.);
  iyStrip = (int)((dist_back + yActive/2.)/yActive*32.);

  //float Random = 2.*gRandom.Rndm() - .5;
  float Random = ran.Rndm();
  float xRecon = ((float)ixStrip + Random)/32.*xActive - xActive/2.;
  //Random = 2.*gRandom.Rndm() - .5;
  Random = ran.Rndm();
  float yRecon = ((float)iyStrip +Random)/32.*yActive - yActive/2.;

  //for 2x2 array of Csi
  if (ixStrip < 16)
    {
      if (iyStrip < 16) ICsI = 0;
      else ICsI = 1;
    }
  else 
    {
      if (iyStrip < 16) ICsI = 3;
      else ICsI = 2;
    }

  //for 3x3 array of CsI
  int ix3 =  (int)((dist_front + xActive/2.)/xActive*3.);
  int iy3 =  (int)((dist_back + yActive/2.)/yActive*3.);
  ICsI3 = 3*ix3+iy3;

  float rRecon[3];
  float rr = 0.;
  for (int i=0;i<3;i++) 
    {
      rRecon[i] = r_center[i] + xRecon*r_front[i] + yRecon*r_back[i];
      rr += pow(rRecon[i],2);
    }
  float r = sqrt(rr);
  thetaRecon = acos(rRecon[2]/r);
  phiRecon = atan2(rRecon[1],rRecon[0]);

  return hitit;

}
//****************************************************
float CTele::getTheta(int ixStrip, int iyStrip)
{
  float Random = ran.Rndm();
  float xRecon = ((float)ixStrip + Random)/32.*xActive - xActive/2.;
  Random = ran.Rndm();
  float yRecon = ((float)iyStrip +Random)/32.*yActive - yActive/2.;
  float rRecon[3];
  float rr = 0.;
  for (int i=0;i<3;i++) 
    {
      rRecon[i] = r_center[i] + xRecon*r_front[i] + yRecon*r_back[i];
      rr += pow(rRecon[i],2);
    }
  float r = sqrt(rr);
  thetaRecon = acos(rRecon[2]/r);
  phiRecon = atan2(rRecon[1],rRecon[0]);
  return thetaRecon;
}

float CTele::getCsICenters(int id0)
{
  ostringstream outstring;
  string name;

  int id = id0;

  outstring.str("");
  outstring << "CsICenters_" << id << ".dat";
  name = outstring.str();

  ofstream ofile(name.c_str());

  float CsI0[3];
  float CsI1[3];
  float CsI2[3];
  float CsI3[3];

  float CsI0r;
  float CsI1r;
  float CsI2r;
  float CsI3r;
  
  CsI0r = 0.;
  CsI1r = 0.;
  CsI2r = 0.;
  CsI3r = 0.;


  for (int i=0;i<3;i++)
    {
      CsI0[i] = r_center[i] - (xActive/4.)*r_front[i] - (yActive/4.)*r_back[i];
      CsI1[i] = r_center[i] - (xActive/4.)*r_front[i] + (yActive/4.)*r_back[i];
      CsI2[i] = r_center[i] + (xActive/4.)*r_front[i] + (yActive/4.)*r_back[i];
      CsI3[i] = r_center[i] + (xActive/4.)*r_front[i] - (yActive/4.)*r_back[i];

      CsI0r += pow(CsI0[i],2);
      CsI1r += pow(CsI1[i],2);
      CsI2r += pow(CsI2[i],2);
      CsI3r += pow(CsI3[i],2);

    }
  
  CsI0r = sqrt(CsI0r);
  CsI1r = sqrt(CsI1r);
  CsI2r = sqrt(CsI2r);
  CsI3r = sqrt(CsI3r);


  float theta0 = acos(CsI0[2]/CsI0r);
  float theta1 = acos(CsI1[2]/CsI1r);
  float theta2 = acos(CsI2[2]/CsI2r);
  float theta3 = acos(CsI3[2]/CsI3r);

  theta0 = theta0*(180./3.14159);
  theta1 = theta1*(180./3.14159);
  theta2 = theta2*(180./3.14159);
  theta3 = theta3*(180./3.14159);

  ofile << "          x     y      z    theta" << endl;
  ofile << id << " " << 0 << " " << CsI0[0] << " " << CsI0[1] << " " << CsI0[2] << " " << theta0 << endl;
  ofile << id << " " << 1 << " " << CsI1[0] << " " << CsI1[1] << " " << CsI1[2] << " " << theta1 << endl;
  ofile << id << " " << 2 << " " << CsI2[0] << " " << CsI2[1] << " " << CsI2[2] << " " << theta2 << endl;
  ofile << id << " " << 3 << " " << CsI3[0] << " " << CsI3[1] << " " << CsI3[2] << " " << theta3 << endl;

  ofile.close();

  return 0.;
}


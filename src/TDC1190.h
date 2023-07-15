#ifndef TDC1190_
#define TDC1190_

#include <cstdlib>
#include <iostream>
using namespace std;

struct tdcStuff
{
  int channel;
  int time;
  int order;
};
/**
 *!\brief readout out of caen 1190 TDC
 *
 * This class is responsibl for reading the data of the 
 * caen 1190 TDC from the data stream and storing the information
 */

class TDC1190
{
 public:
  TDC1190(int depth, int referenceChannel, int Nchannels);
  ~TDC1190();
  unsigned short* read(unsigned short *point);
  void readGlobalHeader(unsigned short word1, unsigned short word2);
  void readTDCheader(unsigned short word1, unsigned short word2);
  void readTDCmeasurement(unsigned short word1, unsigned short word2);
  void readTDCtrailer(unsigned short word1, unsigned short word2);
  void readTDCerror(unsigned short word1, unsigned short word2);
  void readTrailer(unsigned short word1, unsigned short word2);
  int eventCount;  //!< read from header
  int geographic;  //!< geographic address
  int bunchID;   //!< id of bunch
  int eventID;  //!< id of event
  int channel;  //!< channel number
  int data;    //!< TDC data
  int leading;  //!< 0=leading edge, 1=trailing edge
  int wordCount; //!< number of words in TDC event
  int errorFlag; //!< flag indicates error
  int TDC;       //!< TDC number in module
  int status;    //!< status
  int depth;     //!< number of mutihits stored
  int referenceChannel; //!< reference channel where trigger is digitized
  int referenceTime;    //!< time of the reference channel
  int Nchannels;        //!< number of channels where information is saved
  int *  order;         //!< order of infor for a particular channel, 1st hit, etc
  tdcStuff * dataOut;  //!< array containg data from  readout channels
  int Ndata; //!< number of data stored

};
#endif

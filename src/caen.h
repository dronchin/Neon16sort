#ifndef CAEN
#define CAEN
#include <cstdlib>
/**
 * !\brief handles the readout from a CAEN ADC,QDC,TDC
 *
 * This class deals with the read out from a number of 
 * CAEN VME modules
 */

class caen
{
 public:
  unsigned short number; // number of converted channels
  unsigned short crate; // crate number
  unsigned short geographic; // geagraphic address of module
  unsigned short  channel[32];
  unsigned short  data[32];
  unsigned short  underflow[32];
  unsigned short  overflow[32];
  bool read(unsigned short *& point);
};
#endif

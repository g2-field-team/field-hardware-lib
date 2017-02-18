//==============================================================================
//
// Title:		Sg382Interface.h
// Description:	This module contains the software for communicating with the 
// Sg382 RF generator
// Author:Ran Hong
//
//==============================================================================

#ifndef __SG382INTERFACE_H__
#define __SG382INTERFACE_H__

namespace Sg382Interface{
  //Globals
  unsigned int Sg382Port    = 5025;
  unsigned int tcpSg382Handle;
  unsigned int tcpTimeout = 1000;
  //==============================================================================
  // Function Prototypes
  //==============================================================================

  // Connection Functions
  int Sg382Connect(const char* deviceIP);
  int Sg382Disconnect();

  // Get commands
  int GetFrequency(double &f);
  int GetAmplitude(double &A);
  int GetStatus(int &Status);

  // Set commands
  int SetFrequency(double f);
  int SetAmplitude(double A);
  int EnableRF();
  int DisableRF();

}
#endif

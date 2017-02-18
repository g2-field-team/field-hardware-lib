//==============================================================================
//
// Title:		Sg382Interface.cxx
// Author:		Ran Hong
//
//==============================================================================

//==============================================================================
// Include files
//==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <string.h>

#include "Sg382Interface.h"
//#include <tcpsupp.h>
#include "TCPDrivers.h"

using namespace TCPDrivers;
using namespace std;

namespace Sg382Interface{
  int tcpCallbackFunction (unsigned handle, int xType, int errCode, void *callbackData) {return 0;};	// Place holder function

  // Connection Functions
  int Sg382Connect(const char* deviceIP)
  {
    int error = 0;

    char DeviceIP[20];
    sprintf(DeviceIP,deviceIP);

    error = ConnectToTCPServer(&tcpSg382Handle, Sg382Port, DeviceIP, tcpCallbackFunction, NULL, tcpTimeout);
    if (error) return error;
    return 0;
  }

  int Sg382Disconnect()
  {
    int error = 0;
    error = DisconnectFromTCPServer(tcpSg382Handle);
    if (error) return error;
    return 0;
  }

  // Get commands
  int GetFrequency(double &f)
  {
    int NByte = 0;
    char buffer[256];
    char cmd[256];
    sprintf(cmd,"FREQ?MHz\r");
    NByte = ClientTCPWrite (tcpSg382Handle, (void *)&cmd, 9,tcpTimeout);
    NByte = ClientTCPRead (tcpSg382Handle, (void *)&buffer, 256,tcpTimeout);
    f=atof(buffer);
    return 0;
  }

  int GetAmplitude(double &A)
  {
    int NByte = 0;
    char buffer[256];
    char cmd[256];
    sprintf(cmd,"AMPR?\r");
    NByte = ClientTCPWrite (tcpSg382Handle, (void *)&cmd, 6,tcpTimeout);
    NByte = ClientTCPRead (tcpSg382Handle, (void *)&buffer, 256,tcpTimeout);
    A=atof(buffer);
    return 0;
  }

  int GetStatus(int &Status){
    int NByte = 0;
    char buffer[256];
    char cmd[256];
    sprintf(cmd,"ENBR?\r");
    NByte = ClientTCPWrite (tcpSg382Handle, (void *)&cmd, 6,tcpTimeout);
    NByte = ClientTCPRead (tcpSg382Handle, (void *)&buffer, 256,tcpTimeout);
    Status=atoi(buffer);
    return 0;
  }

  // Set commands
  int SetFrequency(double f)
  {
    int NByte = 0;
    char buffer[256];
    char cmd[256];
    sprintf(cmd,"FREQ %f MHz\r",f);
    string Cmd = string(cmd);
    NByte = ClientTCPWrite (tcpSg382Handle, (void *)&cmd, Cmd.size(),tcpTimeout);
    return 0;
  }

  int SetAmplitude(double A)
  {
    int NByte = 0;
    char buffer[256];
    char cmd[256];
    char cmd2[256];
    sprintf(cmd,"AMPR %f\r",A);
    string Cmd = string(cmd);
    NByte = ClientTCPWrite (tcpSg382Handle, (void *)&cmd, Cmd.size(),tcpTimeout);
    return 0;
  }

  int EnableRF()
  {
    int NByte = 0;
    char buffer[256];
    char cmd[256];
    sprintf(cmd,"ENBR 1\r");
    NByte = ClientTCPWrite (tcpSg382Handle, (void *)&cmd, 7,tcpTimeout);
    return 0;
  }

  int DisableRF()
  {
    int NByte = 0;
    char buffer[256];
    char cmd[256];
    sprintf(cmd,"ENBR 0\r");
    NByte = ClientTCPWrite (tcpSg382Handle, (void *)&cmd, 7,tcpTimeout);
    return 0;
  }
}

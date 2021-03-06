//==============================================================================
//
// Title:               TCPDrivers.h
// Description: 	This file contains the declarations of TCP functios needed by Device.c
// Author:              Ran Hong
//
//==============================================================================

#ifndef __TCPDRIVERS_H__
#define __TCPDRIVERS_H__
#include <stdlib.h>

namespace TCPDrivers{
  enum TCPError {
    kTCP_NoError			=0,
    kTCP_UnableToRegisterService	=-1,
    kTCP_UnableToEstablishConnection	=-2,
    kTCP_ExistingServer			=-3,
    kTCP_FailedToConnect		=-4,
    kTCP_ServerNotRegistered		=-5,
    kTCP_TooManyConnections		=-6,
    kTCP_ReadFailed			=-7,
    kTCP_WriteFailed			=-8,
    kTCP_InvalidParameter		=-9,
    kTCP_OutOfMemory			=-10,
    kTCP_TimeOutErr			=-11,
    kTCP_NoConnectionEstablished	=-12,
    kTCP_GeneralIOErr			=-13,
    kTCP_ConnectionClosed		=-14,
    kTCP_UnableToLoadWinsockDLL		=-15,
    kTCP_IncorrectWinsockDLLVersion	=-16,
    kTCP_NetworkSubsystemNotReady	=-17,
    kTCP_ConnectionsStillOpen		=-18,
    kTCP_DisconnectPending		=-19,
    kTCP_InfoNotAvailable		=-20,
    kTCP_HostAddressNotFound		=-21
  };


//Define tcpFuncPtr
typedef int (*tcpFuncPtr)(unsigned, int, int, void *);

//Connect TCP server
int ConnectToTCPServer (unsigned int *conversationHandle, unsigned int portNumber, const char* serverHostName, tcpFuncPtr callbackFunction=NULL, void *callbackData = NULL, unsigned int timeOut=100);

//Disconnect from TCP server
int DisconnectFromTCPServer (unsigned int conversationHandle);

//Read from TCP server
int ClientTCPRead (unsigned int conversationHandle, void *dataBuffer, size_t dataSize, unsigned int timeOut=100);

//Write to TCP server
int ClientTCPWrite (unsigned int conversationHandle, void *dataPointer, int dataSize, unsigned int timeOut1=100);
}

#endif

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
    kTCP_NoError,
    kTCP_UnableToRegisterService,
    kTCP_UnableToEstablishConnection,
    kTCP_ExistingServer,
    kTCP_FailedToConnect,
    kTCP_ServerNotRegistered,
    kTCP_TooManyConnections,
    kTCP_ReadFailed,
    kTCP_WriteFailed,
    kTCP_InvalidParameter,
    kTCP_OutOfMemory,
    kTCP_TimeOutErr,
    kTCP_NoConnectionEstablished,
    kTCP_GeneralIOErr,
    kTCP_ConnectionClosed,
    kTCP_UnableToLoadWinsockDLL,
    kTCP_IncorrectWinsockDLLVersion,
    kTCP_NetworkSubsystemNotReady,
    kTCP_ConnectionsStillOpen,
    kTCP_DisconnectPending,
    kTCP_InfoNotAvailable,
    kTCP_HostAddressNotFound
  };


//Define tcpFuncPtr
typedef int (*tcpFuncPtr)(unsigned, int, int, void *);

//Connect TCP server
int ConnectToTCPServer (unsigned int *conversationHandle, unsigned int portNumber, char serverHostName[], tcpFuncPtr callbackFunction=NULL, void *callbackData = NULL, unsigned int timeOut=100);

//Disconnect from TCP server
int DisconnectFromTCPServer (unsigned int conversationHandle);

//Read from TCP server
int ClientTCPRead (unsigned int conversationHandle, void *dataBuffer, size_t dataSize, unsigned int timeOut=100);

//Write to TCP server
int ClientTCPWrite (unsigned int conversationHandle, void *dataPointer, int dataSize, unsigned int timeOut1=100);
}

#endif
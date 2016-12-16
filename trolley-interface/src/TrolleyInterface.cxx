//==============================================================================
//
// Title:		Device.c
// Description:	<removed due to outdatedness>
// Author:		Andrew Kreps, Michael Oberling
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

#include "TrolleyInterface.h"
//#include <tcpsupp.h>
#include "TCPDrivers.h"

using namespace TCPDrivers;
using namespace std;

namespace TrolleyInterface{
  //==============================================================================
  // Variables
  //==============================================================================

  // Shared TCP/UDP Connection Variables
  unsigned int	ControlPort0	= 55001;
  unsigned int	ControlPort1	= 55002;
  unsigned int	DataPort	= 55010;
  char	DeviceIP[20]	= "XXX.XXX.XXX.XXX";

  // TCP Connection Variables
  unsigned int tcpControlHandle;
  //unsigned int	tcpControlHandle2;
  int tcpControlCallbackFunction (unsigned handle, int xType, int errCode, void *callbackData) {return 0;};	// Place holder function
  unsigned int tcpControlTimeout 		= 5000;
  unsigned int tcpDataHandle;
  ifstream FileStream;
  bool ReadFromFile = false;
  int tcpDataCallbackFunction (unsigned handle, int xType, int errCode, void *callbackData) {return 0;};   	// Place holder function
  unsigned int tcpDataTimeout 			= 5000;

  // Data Path Variables
  //  int dataSynced = 0;
  typedef enum _DATA_SYNC_STATE
  {
    DSS_WORD_8000_BYTE_0_TEST,
    DSS_WORD_8000_BYTE_1_TEST,
    DSS_WORD_TEST,
    DSS_WORD_7FFF,
    DSS_WORD_8000,
    DSS_WORD_AAAA,
    DSS_WORD_SUCCESS
  } DATA_SYNC_STATE;
  DATA_SYNC_STATE data_sync_state = DSS_WORD_7FFF;

  // For diagnostics only
  //DWORD	dataLost		= 0;
  int dataLost = 0;


  //==============================================================================
  // Connection Functions
  //==============================================================================

  int DeviceConnect (const char* deviceIP)
  {
    int error = 0;
    ReadFromFile = false;

    sprintf(DeviceIP,deviceIP);

    // First open the slow control connection.
    error = ConnectToTCPServer(&tcpControlHandle, ControlPort0, DeviceIP, tcpControlCallbackFunction, NULL, tcpControlTimeout);
    if (error) return errorCommConnect;
    cout <<"Control Connection OK"<<endl;

    // It's perfectly fine to open multiple slow control connections to the SSP.
    // However, due to a limitation in the current firmware only two connection are supported and they must connected to ports 55001, and 55002.
    // In our code the monitor process will use this second connection, and the GUI controls, and test procedures will use the first.
    //
    // Commented out for distribution
    //error = ConnectToTCPServer(&tcpControlHandle2, ControlPort1, DeviceIP, tcpControlCallbackFunction, NULL, tcpControlTimeout);
    // if (error) return errorCommConnect;

    //Purge data before connect
    DevicePurgeData();

    // Open the event data connection.
    error = ConnectToTCPServer(&tcpDataHandle, DataPort, DeviceIP, tcpDataCallbackFunction, NULL, tcpDataTimeout);
    if (error) return errorCommConnect;
    cout <<"Data Connection OK"<<endl;

    // Set the event data interface to Ethernet
    //  DeviceWrite(trolleyReg.eventDataInterfaceSelect, 0x00000001);
    //For testing IO
    /*
       char buffer[256] = "This is a test.\n";
       ClientTCPWrite(tcpControlHandle,buffer,strlen(buffer),5000);
       ClientTCPWrite(tcpDataHandle,buffer,strlen(buffer),5000);
       char inbuffer[256];
       ClientTCPRead(tcpControlHandle,inbuffer,256,5000);
       cout << inbuffer<<endl;
       ClientTCPRead(tcpDataHandle,inbuffer,256,5000);
       cout << inbuffer<<endl;
     */

    // Set the event data interface to Ethernet
    DeviceWrite(trolleyReg.eventDataInterfaceSelect, 0x00000001);
    return errorNoError;
  }

  int DeviceDisconnect (void)
  {
    int error = 0;

    error |= DisconnectFromTCPServer(tcpControlHandle);
    //error |= DisconnectFromTCPServer(tcpControlHandle2);
    error |= DisconnectFromTCPServer(tcpDataHandle); 
    if (error) return errorCommDisconnect;

    return errorNoError;
  }			

  int FileOpen(const char * filename)
  {
    ReadFromFile = true;
    FileStream.open(filename,ios::binary);
    if (!FileStream.is_open()){
      return errorDataConnect;
    }
    return errorNoError;
  }

  int FileClose()
  {
    ReadFromFile = false;
    FileStream.close();
    return errorNoError;
  }

  int DevicePurgeComm (void)
  {
    int		error = 0;

    error = DisconnectFromTCPServer(tcpControlHandle);
    if (error) return errorCommDisconnect;

    error = ConnectToTCPServer(&tcpControlHandle, ControlPort0, DeviceIP, tcpControlCallbackFunction, NULL, tcpControlTimeout);
    if (error) return errorCommConnect;

    return errorNoError;
  }

  int DevicePurgeData (void)
  {
    int error = errorNoError;
    unsigned int TCPdata[1000];
    int rxStatus = 0;
    do
    {
      rxStatus = ClientTCPRead(tcpDataHandle, (unsigned int*)(TCPdata), sizeof(TCPdata), 100);
    } while(rxStatus > 0);
    return error;
  }

  // This is the software's data interface function to the TCP/IP driver.
  int DataReceive (void* data)
  {
    int	error	 = errorUnknown;
    int	rxStatus = 0;
    unsigned int dataExpected	= 0;
    unsigned int dataReceived	= 0;
    unsigned int packetLength	= 0;
    unsigned int payloadLength	= 0;
    unsigned short int preable_sync_buffer = 0;

    dataReceived = 0;

    data_sync_state = DSS_WORD_TEST;	// Initially assume that the data is aligned, at the word level.  Then check for the
    // the preamble sequence.  If it is not detected revert to byte-wise alignment.
    do /* sync check loop */
    {
      if((data_sync_state == DSS_WORD_8000_BYTE_0_TEST) || (data_sync_state == DSS_WORD_8000_BYTE_1_TEST))
      {
	dataExpected = sizeof(unsigned char);
      }
      else
      {
	dataExpected = sizeof(unsigned short int);
      }

      // Grab the data from the TCP driver
      if (ReadFromFile){
	if (FileStream.eof()){
	  return errorEOF;
	}
	FileStream.read((char *)&preable_sync_buffer,dataExpected);
	if (FileStream.eof()){
	  return errorEOF;
	}
	if (FileStream.fail()){
	  return errorFileFail;
	}
	rxStatus = dataExpected;
	error = errorNoError;
      }else{
	rxStatus = ClientTCPRead(tcpDataHandle, &preable_sync_buffer, dataExpected, tcpDataTimeout);
	error = ClientTCPRead_ErrorCheck(rxStatus);
      }
      if (error != errorNoError) break;

      dataReceived += rxStatus;
      dataExpected -= rxStatus;

      // check if we received all the expected data
      if (dataExpected == 0)
      {
	switch (data_sync_state)
	{
	  // Check for the first byte of the first preamble word. "7F(FF)"
	  case DSS_WORD_8000_BYTE_0_TEST:
	    if ((preable_sync_buffer & 0x00FF) == 0x0000)
	    {
	      data_sync_state = DSS_WORD_8000_BYTE_1_TEST;
	    }
	    else
	    {
	      // last byte was not part of the expected start of header word.
	      // reset receive data count
	      dataLost += dataReceived;
	      dataReceived = 0;
	    }
	    break;

	  case DSS_WORD_8000_BYTE_1_TEST:
	    if ((preable_sync_buffer & 0x00FF) == 0x0080)
	    {
	      // possible word alignment found
	      data_sync_state = DSS_WORD_7FFF;
	    }
	    else
	    {
	      data_sync_state = DSS_WORD_8000_BYTE_0_TEST;
	      // last byte was not part of the expected start of header word.
	      // reset receive data count
	      dataLost += dataReceived;
	      dataReceived = 0;
	    }
	    break;

	  case DSS_WORD_TEST:
	    if (preable_sync_buffer == 0x7FFF)
	    {
	      data_sync_state = DSS_WORD_8000;
	    }
	    else if (preable_sync_buffer == 0x8000)
	    {
	      data_sync_state = DSS_WORD_7FFF;
	    }
	    else
	    {
	      data_sync_state = DSS_WORD_8000_BYTE_0_TEST;
	      // Throw this data away and increment lost data count
	      dataLost += dataReceived;
	      dataReceived = 0;
	    }
	    break;

	  case DSS_WORD_7FFF:
	    if (preable_sync_buffer == 0x7FFF)
	    {
	      data_sync_state = DSS_WORD_8000;
	    }
	    else
	    {
	      data_sync_state = DSS_WORD_8000_BYTE_0_TEST;
	      // Throw this data away and increment lost data count
	      dataLost += dataReceived;
	      dataReceived = 0;
	    }
	    break;

	  case DSS_WORD_8000:
	    if (preable_sync_buffer == 0x8000)
	    {
	      data_sync_state = DSS_WORD_AAAA;
	    }
	    else
	    {
	      data_sync_state = DSS_WORD_8000_BYTE_0_TEST;
	      // Throw this data away and increment lost data count
	      dataLost += dataReceived;
	      dataReceived = 0;
	    }
	    break;

	  case DSS_WORD_AAAA:
	    if (preable_sync_buffer == 0xAAAA)
	    {
	      data_sync_state = DSS_WORD_SUCCESS;
	    }
	    else if (preable_sync_buffer == 0x7FFF)
	    {
	      data_sync_state = DSS_WORD_8000;
	    }
	    else
	    {
	      data_sync_state = DSS_WORD_8000_BYTE_0_TEST;
	      // Throw this data away and increment lost data count
	      dataLost += dataReceived;
	      dataReceived = 0;
	    }
	    break;

	  default:
	    error = errorUnknown; 
	    break;
	}
      }
    } while((error == errorNoError) && (data_sync_state != DSS_WORD_SUCCESS));

    if (error) return error;

    ((unsigned short int*)data)[0] = 0x7FFF;
    ((unsigned short int*)data)[1] = 0x8000;
    ((unsigned short int*)data)[2] = 0x7FFF;
    ((unsigned short int*)data)[3] = 0x8000;
    ((unsigned short int*)data)[4] = 0x7FFF;
    ((unsigned short int*)data)[5] = 0x8000;
    ((unsigned short int*)data)[6] = 0xAAAA;

    dataReceived = 7 * sizeof(unsigned short);

    // Calculate the event packet payload size

    // Fetch Event Packet Length
    dataExpected = sizeof(unsigned int);
    do
    {
      if (ReadFromFile){
	if (FileStream.eof()){
	  return errorEOF;
	}
	FileStream.read((char *)(&(((unsigned char*)data)[dataReceived])),dataExpected);
	if (FileStream.eof()){
	  return errorEOF;
	}
	if (FileStream.fail()){
	  return errorFileFail;
	}
	rxStatus = dataExpected;
	error = errorNoError;
      }else{
	rxStatus = ClientTCPRead(tcpDataHandle, (void*)(&(((unsigned char*)data)[dataReceived])), dataExpected, tcpDataTimeout);
	error = ClientTCPRead_ErrorCheck(rxStatus);
      }
      if (error != errorNoError) break;

      dataReceived += rxStatus; 
      dataExpected -= rxStatus;
    } while(dataExpected != 0);

    packetLength	= (*((unsigned int*)(&(((unsigned short int*)data)[7])))) * sizeof(unsigned char);
    payloadLength	= packetLength - EVENT_HEADER_SIZE;

    // Basic check of header data values
    if (packetLength < EVENT_HEADER_SIZE) 
    {
      error = errorDataLength;	// Reported size of packet is smaller than a header!
    } 
    else if (payloadLength > MAX_PAYLOAD_DATA)
    {
      error = errorDataLength;	// Reported size of waveform is too large!
    }

    if (error) return error; 

    dataExpected = packetLength - dataReceived;

    // Fetch Event Payload
    do
    {
      if (ReadFromFile){
	if (FileStream.eof()){
	  return errorEOF;
	}
	FileStream.read((char *)(&(((unsigned char*)data)[dataReceived])),dataExpected);
	if (FileStream.eof()){
	  return errorEOF;
	}
	if (FileStream.fail()){
	  return errorFileFail;
	}
	rxStatus = dataExpected;
	error = errorNoError;
      }else{
	rxStatus = ClientTCPRead(tcpDataHandle, (void*)(&(((unsigned char*)data)[dataReceived])), dataExpected, tcpDataTimeout);
	error = ClientTCPRead_ErrorCheck(rxStatus);
      }
      if (error != errorNoError) break;

      dataReceived += rxStatus; 
      dataExpected -= rxStatus;
    } while(dataExpected != 0);

    if (error) return error; 

    return dataReceived;
  }


  // This is the software's slow control interface function to the TCP/IP driver.
  int SendReceive (Ctrl_Packet* tx, Ctrl_Packet* rx)
  {

    int error = errorUnknown;
    int rxStatus;
    int txStatus;

    txStatus = ClientTCPWrite(tcpControlHandle, tx, tx->header.length, tcpControlTimeout);
    error = ClientTCPWrite_ErrorCheck(txStatus);
    if (error != errorNoError) return error;
    if(txStatus != tx->header.length)
    {
      return errorCommSend;
    }

    // The first word is the total length of the packet.
    rxStatus = ClientTCPRead(tcpControlHandle, &(rx->header.length), sizeof(unsigned int), tcpControlTimeout);
    error = ClientTCPRead_ErrorCheck(rxStatus);
    if (error != errorNoError) return error;

    if ((rx->header.length > sizeof(Ctrl_Packet)) || (rx->header.length < sizeof(Ctrl_Header)))
    {	
      return errorCommReceive;
    }

    // Read the rest of the data packet into the allocated receive buffer.
    rxStatus = ClientTCPRead(tcpControlHandle, &(((unsigned int*)(rx))[1]), rx->header.length - sizeof(unsigned int), tcpControlTimeout);
    error = ClientTCPRead_ErrorCheck(rxStatus);
    return error;
  }

  int ClientTCPWrite_ErrorCheck (int txStatus)
  {
    int error = errorUnknown;

    if (txStatus > 0)
    {
      // if successful, txStatus is the number of bytes received
      error = errorNoError;
    }
    else
    {
      // if not translate the error status
      switch(txStatus)
      {
	case 0:					error = errorCommSendZero;		break;
	case kTCP_TimeOutErr:	error = errorCommSendTimeout;	break;
	default:				error = errorCommSend;			break;
      }
    }

    return error;
  }

  int ClientTCPRead_ErrorCheck (int rxStatus)
  {
    int error = errorUnknown;

    if (rxStatus > 0)
    {
      // if successful, rxStatus is the number of bytes received
      error = errorNoError;
    }
    else
    {
      // if not translate the error status
      switch(rxStatus)
      {
	case 0:					error = errorCommReceiveZero;		break;
	case kTCP_TimeOutErr:	error = errorCommReceiveTimeout;	break;
	default:				error = errorCommReceive;			break;
      }
    }

    return error;
  }


  //==============================================================================
  // Command Functions
  //==============================================================================

  int DeviceRead (unsigned int address, unsigned int* value)
  {
    int 		error = 0;
    Ctrl_Packet	rx;
    Ctrl_Packet	tx;
    int			rxSizeExpected = 0;

    tx.header.length	= sizeof(Ctrl_Header);
    tx.header.address	= address;
    tx.header.command	= cmdRead;
    tx.header.size		= 1;
    tx.header.status	= statusNoError;
    rxSizeExpected		= sizeof(Ctrl_Header) + sizeof(unsigned int);

    error = SendReceive(&tx, &rx);
    if (error == 0) {
      if(tx.header.address != rx.header.address)
      {
	error = errorCommReceive;
      }
    }

    if (error == 0) {
      // No Error, return data
      *value = rx.data[0];
    } else {
      // Error, return zero
      *value = 0;
    }
    return error;
  }

  int DeviceWrite (unsigned int address, unsigned int value)
  {
    int 		error = 0;
    Ctrl_Packet	rx;
    Ctrl_Packet	tx;
    int			rxSizeExpected = 0;

    tx.header.length	= sizeof(Ctrl_Header) + sizeof(unsigned int);
    tx.header.address	= address;
    tx.header.command	= cmdWrite;
    tx.header.size		= 1;
    tx.header.status	= statusNoError;
    tx.data[0]			= value;
    rxSizeExpected		= sizeof(Ctrl_Header);

    error = SendReceive(&tx, &rx);
    if (error == 0) {
      if(tx.header.address != rx.header.address)
      {
	error = errorCommSend;
      }
    }

    return error;
  }

  int DeviceReadMask (unsigned int address, unsigned int mask, unsigned int* value)
  {
    int 		error = 0;
    Ctrl_Packet	rx;
    Ctrl_Packet	tx;
    int			rxSizeExpected = 0;

    tx.header.length	= sizeof(Ctrl_Header) + sizeof(unsigned int);
    tx.header.address	= address;
    tx.header.command	= cmdReadMask;
    tx.header.size		= 1;
    tx.header.status	= statusNoError;
    tx.data[0]			= mask;
    rxSizeExpected		= sizeof(Ctrl_Header) + sizeof(unsigned int);

    error = SendReceive(&tx, &rx);
    if (error == 0) {
      if(tx.header.address != rx.header.address)
      {
	error = errorCommReceive;
      }
    }

    if (error == 0) {
      // No Error, return data
      *value = rx.data[0];
    } else {
      // Error, return zero
      *value = 0;
    }

    return error;
  }

  int DeviceWriteMask (unsigned int address, unsigned int mask, unsigned int value)
  {
    int 		error = 0;
    Ctrl_Packet	rx;
    Ctrl_Packet	tx;
    int			rxSizeExpected = 0;

    tx.header.length	= sizeof(Ctrl_Header) + (sizeof(unsigned int) * 2);
    tx.header.address	= address;
    tx.header.command	= cmdWriteMask;
    tx.header.size		= 1;
    tx.header.status	= statusNoError;
    tx.data[0]			= mask;
    tx.data[1]			= value;
    rxSizeExpected		= sizeof(Ctrl_Header) + sizeof(unsigned int); 

    error = SendReceive(&tx, &rx);
    if (error == 0) {
      if(tx.header.address != rx.header.address)
      {
	error = errorCommSend;
      }					 
    }

    return error;
  }

  int DeviceSet (unsigned int address, unsigned int mask)
  {
    return DeviceWriteMask(address, mask, 0xFFFFFFFF);
  }

  int DeviceClear (unsigned int address, unsigned int mask)
  {
    return DeviceWriteMask(address, mask, 0x00000000);
  }

  int DeviceArrayRead (unsigned int address, unsigned int size, unsigned int* data)
  {
    int 		i = 0;
    int 		error = 0;
    Ctrl_Packet	rx;
    Ctrl_Packet	tx;
    int			rxSizeExpected = 0;

    tx.header.length	= sizeof(Ctrl_Header);
    tx.header.address	= address;
    tx.header.command	= cmdArrayRead;
    tx.header.size		= size;
    tx.header.status	= statusNoError;
    rxSizeExpected		= sizeof(Ctrl_Header) + (sizeof(unsigned int) * size);

    error = SendReceive(&tx, &rx);
    if (error == 0) {
      if(tx.header.address != rx.header.address)
      {
	error = errorCommReceive;
      }
      // No Error, return data
      memcpy(data, rx.data, size * sizeof(unsigned int));
    } else {
      // Error, return zeros
      for (i = 0; i < size; i++) {
	data[i] = 0;
      }
    }


    return error;
  }

  int DeviceArrayWrite (unsigned int address, unsigned int size, unsigned int* data)
  {
    int 		i = 0;
    int 		error = 0;
    Ctrl_Packet	rx;
    Ctrl_Packet	tx;
    int			rxSizeExpected = 0;

    tx.header.length	= sizeof(Ctrl_Header) + (sizeof(unsigned int) * size);
    tx.header.address	= address;
    tx.header.command	= cmdArrayWrite;
    tx.header.size		= size;
    tx.header.status	= statusNoError;
    rxSizeExpected		= sizeof(Ctrl_Header);

    for (i = 0; i < size; i++) {
      tx.data[i] = data[i];
    }

    error = SendReceive(&tx, &rx);
    if (error == 0) {
      if(tx.header.address != rx.header.address)
      {
	error = errorCommSend;
      }
    }

    return error;
  }

  void RegInit (void)
  {
    // NOTE: All comments about default values, read masks, and write masks are current as of 2/11/2014

    // Registers in the Zynq ARM		Address				Address		Default Value	Read Mask		Write Mask		Code Name
    trolleyReg.comm_arm_fw_build = 0x00000010;
    trolleyReg.IP4Address	 = 0x00000100;
    trolleyReg.IP4Netmask	 = 0x00000104;
    trolleyReg.IP4Gateway	 = 0x00000108;
    trolleyReg.MACAddressLSB	 = 0x00000110;
    trolleyReg.MACAddressMSB	 = 0x00000114;
    trolleyReg.EthernetReset	 = 0x00000180;
    trolleyReg.RestoreSelect	 = 0x00000200;
    trolleyReg.Restore		 = 0x00000204;
    trolleyReg.PurgeDDR		 = 0x00000300;
    trolleyReg.TCPSendBlockSize	 = 0x00000400;


    // Registers in the Zynq FPGA		Address				Address		Default Value	Read Mask		Write Mask		VHDL Name
    trolleyReg.comm_fpga_fw_build 	= 0x40000010;   //	X"010",		X"00000001",	X"FFFFFFFF",	X"00000000",
    trolleyReg.eventDataInterfaceSelect	= 0x40000020;	//	X"020",		X"00000000",	X"00000001",	X"00000001",	 
    trolleyReg.eventDataControl		= 0x40000144;	//	X"144",		X"0020001F",	X"FFFFFFFF",	X"0033001F",	reg_event_data_control
    trolleyReg.eventDataStatus		= 0x40000190;	//	X"190",		X"00000000",	X"FFFFFFFF",	X"00000000",	regin_event_data_status
  }

}//End namespace Trolley Interface

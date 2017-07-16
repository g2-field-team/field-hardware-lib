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

    //Purge data before connect
    DeviceWriteMask(0x40000944,0x00000001,0x00000001);
    DevicePurgeData();

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
      //rxStatus = ClientTCPRead(tcpDataHandle, (unsigned int*)(TCPdata), sizeof(TCPdata), tcpDataTimeout);
      rxStatus = ClientTCPRead(tcpDataHandle, (unsigned int*)(TCPdata), sizeof(TCPdata), 500);
    } while(rxStatus > 0);
    return error;
  }

  // This is the software's data interface function to the TCP/IP driver.
  int DataReceive (void (*const a_frame), void (*const b_frame), unsigned int (*const a_frame_length), unsigned int (*const b_frame_length))
  {
    int					error				= errorUnknown;
    int					rxStatus			= 0;
    unsigned int		dataExpected		= 0;
    unsigned int		dataReceived		= 0;
    unsigned int		packetLength		= 0;
    unsigned int		payloadLength		= 0;
    unsigned short int	preable_sync_buffer	= 0;
    unsigned short int	sint_value			= 0;
    unsigned short int  int_value			= 0;
    int 				i;
    unsigned int 		a_length;
    unsigned int 		b_length;
    a_length = 0;
    b_length = 0;
    *a_frame_length = 0;
    *b_frame_length = 0;
    dataReceived	= 0;
    data_sync_state = DSS_WORD_TEST;	// Initially assume that the data is aligned, at the word level.  Then check for the

    // the preamble sequence.  If it is not detected revert to byte-wise alignment.

    do  //sync check loop 
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
	    else if (preable_sync_buffer == 0xBBBB)
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

    packetLength = 0;
    if (preable_sync_buffer == 0xAAAA)
    {
      ((unsigned short int*)a_frame)[0] = 0x7FFF;
      ((unsigned short int*)a_frame)[1] = 0x8000;
      ((unsigned short int*)a_frame)[2] = 0x7FFF;
      ((unsigned short int*)a_frame)[3] = 0x8000;
      ((unsigned short int*)a_frame)[4] = 0x7FFF;
      ((unsigned short int*)a_frame)[5] = 0x8000;
      ((unsigned short int*)a_frame)[6] = 0xAAAA;
      a_length = 7 * sizeof(unsigned short);
      // Calculate the event packet payload size
      // Fetch Event Packet Length
      dataExpected = EVENT_HEADER_SIZE - a_length;
      do
      {
	if (ReadFromFile){
	  if (FileStream.eof()){
	    return errorEOF;
	  }
	  FileStream.read((char *)(&(((unsigned char*)a_frame)[a_length])),dataExpected);
	  if (FileStream.eof()){
	    return errorEOF;
	  }
	  if (FileStream.fail()){
	    return errorFileFail;
	  }
	  rxStatus = dataExpected;
	  error = errorNoError;
	}else{
	  rxStatus = ClientTCPRead(tcpDataHandle, (void*)(&(((unsigned char*)a_frame)[a_length])), dataExpected, tcpDataTimeout);
	  error = ClientTCPRead_ErrorCheck(rxStatus);
	}
	if (error != errorNoError) break;
	a_length += rxStatus; 
	dataExpected -= rxStatus;
      } while(dataExpected != 0);
      packetLength	= (*((unsigned int*)(&(((unsigned short int*)a_frame)[7])))) * sizeof(unsigned char);
      if ((unsigned short int)(((unsigned short int*)a_frame)[14]) > 0)
      {
	packetLength	-= 2;
      }							
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
      dataExpected = packetLength - a_length;
      // Fetch Event Payload
      do
      {
	if (ReadFromFile){
	  if (FileStream.eof()){
	    return errorEOF;
	  }
	  FileStream.read((char *)(&(((unsigned char*)a_frame)[a_length])),dataExpected);
	  if (FileStream.eof()){
	    return errorEOF;
	  }
	  if (FileStream.fail()){
	    return errorFileFail;
	  }
	  rxStatus = dataExpected;
	  error = errorNoError;
	}else{
	  rxStatus = ClientTCPRead(tcpDataHandle, (void*)(&(((unsigned char*)a_frame)[a_length])), dataExpected, tcpDataTimeout);
	  error = ClientTCPRead_ErrorCheck(rxStatus);
	}
	if (error != errorNoError) break;
	a_length += rxStatus; 
	dataExpected -= rxStatus;
      } while(dataExpected != 0);


      if (error) return error;
    }
    else
    {
      ((unsigned short int*)b_frame)[0] = 0x8000;
      ((unsigned short int*)b_frame)[1] = 0x7FFF;
      ((unsigned short int*)b_frame)[2] = 0x8000;
      ((unsigned short int*)b_frame)[3] = 0xBBBB;
      b_length = 4 * sizeof(unsigned short);	
    }
    packetLength =  36 * sizeof(unsigned short int);
    dataExpected = packetLength - b_length;
    // 
    do
    {
      if (ReadFromFile){
	if (FileStream.eof()){
	  return errorEOF;
	}
	FileStream.read((char *)(&(((unsigned char*)b_frame)[b_length])),dataExpected);
	if (FileStream.eof()){
	  return errorEOF;
	}
	if (FileStream.fail()){
	  return errorFileFail;
	}
	rxStatus = dataExpected;
	error = errorNoError;
      }else{
	rxStatus = ClientTCPRead(tcpDataHandle, (void*)(&(((unsigned char*)b_frame)[b_length])), dataExpected, tcpDataTimeout);
	error = ClientTCPRead_ErrorCheck(rxStatus);
      }
      if (error != errorNoError) break;
      b_length += rxStatus; 
      dataExpected -= rxStatus;
    } while(dataExpected != 0);
    if (error) return error;
    packetLength += 2*(*((unsigned short int*)(&(((unsigned char*)b_frame)[b_length-64])))) * sizeof(unsigned short int);
    dataExpected = packetLength - b_length;
    if (dataExpected > 0) 
    {
      //
      do
      {
	if (ReadFromFile){
	  if (FileStream.eof()){
	    return errorEOF;
	  }
	  FileStream.read((char *)(&(((unsigned char*)b_frame)[b_length])),dataExpected);
	  if (FileStream.eof()){
	    return errorEOF;
	  }
	  if (FileStream.fail()){
	    return errorFileFail;
	  }
	  rxStatus = dataExpected;
	  error = errorNoError;
	}else{
	  rxStatus = ClientTCPRead(tcpDataHandle, (void*)(&(((unsigned char*)b_frame)[b_length])), dataExpected, tcpDataTimeout);
	  error = ClientTCPRead_ErrorCheck(rxStatus);
	}
	if (error != errorNoError) break;
	b_length += rxStatus; 
	dataExpected -= rxStatus;
      } while(dataExpected != 0);
      if (error) return error;
    }
    *a_frame_length = a_length;
    *b_frame_length = b_length;
    return error;
  }
  

  void DataPreProcess (const void (*const a_frame), const void (*const b_frame), A_FRAME_INFO (*const a_frame_info), B_FRAME_INFO (*const b_frame_info))
  {
    const unsigned char (*const a_frame_uchar)	= (const unsigned char (*const))(a_frame);
    const unsigned short int (*const a_frame_ushort) = (const unsigned short int (*const))(a_frame);
    const unsigned long  int (*const a_frame_ulong) = (const unsigned long int (*const))(a_frame);
    const unsigned long long int (*const a_frame_ulonglong) = (const unsigned long long int (*const))(a_frame);
    const unsigned short int (*const b_frame_ushort) = (const unsigned short int (*const))(b_frame);
    const unsigned long int (*const b_frame_ulong) = (const unsigned long int (*const))(b_frame);
    const unsigned long long int (*const b_frame_ulonglong) = (const unsigned long long int (*const))(b_frame);

    double c_value[7];
    double d_value[2];
    double dt_value;
    double temp_value;
    double temp_c_value;
    double off_value;
    double sens_value;
    double temp_correction_value;
    double off_correction_value;
    double sens_correction_value;
    double temp_2nd_value;
    double temp_c_2nd_value;
    double off_2nd_value;
    double sens_2nd_value;
    double p_2nd_value;
    double pbar_2nd_value;

    if (a_frame != NULL)
    {
      a_frame_info->frame_length = (*((unsigned int*)(&(a_frame_ushort[7]))));
      a_frame_info->frame_number = (*((unsigned int*)(&(a_frame_ushort[9]))));
      a_frame_info->probe_number = a_frame_uchar[22];
      a_frame_info->power_status = a_frame_uchar[23];
      a_frame_info->nmr_samples = a_frame_ushort[12];
      a_frame_info->barcode_samples = a_frame_ushort[13];
      a_frame_info->flash_data_length = a_frame_ushort[14];
      a_frame_info->voltage_monitor_min[0] = 5.0 * (float)(a_frame_ushort[15]) / 65536.0 * 2.0;
      a_frame_info->voltage_monitor_max[0] = 5.0 * (float)(a_frame_ushort[16]) / 65536.0 * 2.0;
      a_frame_info->voltage_monitor_min[1] = 5.0 * (float)(a_frame_ushort[17]) / 65536.0;
      a_frame_info->voltage_monitor_max[1] = 5.0 * (float)(a_frame_ushort[18]) / 65536.0;
      a_frame_info->temperature_monitor[0] = (float)(a_frame_ushort[19]) / 128.0;
      a_frame_info->temperature_monitor[1] = (float)(a_frame_ushort[20]) / 128.0;
      a_frame_info->temperature_monitor[2] = (float)(a_frame_ushort[21]);	// No sensor
      a_frame_info->temperature_monitor[3] = (float)(a_frame_ushort[22]);	// No sensor
      a_frame_info->press_cal[0] = a_frame_ushort[23];
      a_frame_info->press_cal[1] = a_frame_ushort[24];             
      a_frame_info->press_cal[2] = a_frame_ushort[25];             
      a_frame_info->press_cal[3] = a_frame_ushort[26];             
      a_frame_info->press_cal[4] = a_frame_ushort[27];             
      a_frame_info->press_cal[5] = a_frame_ushort[28];             
      a_frame_info->press_cal[6] = a_frame_ushort[29];             
      a_frame_info->press_raw_press = a_frame_ulong[15];
      a_frame_info->press_raw_temp = a_frame_ulong[16];
      a_frame_info->timestamp_cycle_start = a_frame_ulonglong[9];
      a_frame_info->timestamp_nmr_start = a_frame_ulonglong[10];       
      a_frame_info->timestamp_barcode_start = a_frame_ulonglong[11];   
      a_frame_info->nmr_waveform_checksum = a_frame_ulong[23];
      a_frame_info->nmr_timestamp_offset = a_frame_ushort[50];
      a_frame_info->bc_config_out = a_frame_ushort[51];        
      a_frame_info->bc_refdac1_out = a_frame_ushort[52];
      a_frame_info->bc_refdac2_out = a_frame_ushort[53];
      a_frame_info->bc_seqfifo_out = a_frame_ushort[54];        
      a_frame_info->bc_refcm_out = a_frame_ushort[55];        
      a_frame_info->rf_power_meas[0] = a_frame_ulong[30];
      a_frame_info->rf_power_meas[1] = a_frame_ulong[31];

      c_value[0] = (double)(a_frame_ushort[23]);
      c_value[1] = (double)(a_frame_ushort[24]);             
      c_value[2] = (double)(a_frame_ushort[25]);             
      c_value[3] = (double)(a_frame_ushort[26]);             
      c_value[4] = (double)(a_frame_ushort[27]);             
      c_value[5] = (double)(a_frame_ushort[28]);             
      c_value[6] = (double)(a_frame_ushort[29]);             
      d_value[0] = (double)(a_frame_ulong[15]);
      d_value[1] = (double)(a_frame_ulong[16]);
      dt_value = d_value[1]-c_value[5]*((double)(1 << 8));
      temp_value = 2000.0+dt_value*c_value[6]/((double)(1 << 23)); 
      temp_c_value = temp_value /100.0;
      off_value = c_value[2]*((double)(1 << 17))+(c_value[4]*dt_value)/((double)(1 << 6)); 
      sens_value = c_value[1]*((double)(1 << 16))+(c_value[3]*dt_value)/((double)(1 << 7));

      temp_2nd_value = temp_value;
      off_2nd_value = off_value;
      sens_2nd_value = sens_value;

      if (temp_c_value >= 20)
      {
	temp_correction_value = 5.0 * (dt_value/((double)((unsigned long long int)(1) << 38))) * (dt_value/((double)((unsigned long long int)(1) << 38)));
	off_correction_value = 0;
	sens_correction_value = 0;
      }
      else if (temp_c_value < -15)
      {
	temp_correction_value = 3.0 * (dt_value/((double)((unsigned long long int)(1) << 33))) * (dt_value/((double)((unsigned long long int)(1) << 33)));    
	off_correction_value = 61.0 * (temp_value-2000.0) * (temp_value-2000.0) / ((double)(1 << 4))+17*(temp_value+1500.0) * (temp_value+1500.0);
	sens_correction_value = 29.0 * (temp_value-2000.0) * (temp_value-2000.0) / ((double)(1 << 4))+9*(temp_value+1500.0) * (temp_value+1500.0); 
      }
      else
      {
	temp_correction_value = 3.0 * (dt_value/((double)((unsigned long long int)(1) << 33))) * (dt_value/((double)((unsigned long long int)(1) << 33)));
	off_correction_value = 61.0 * (temp_value-2000.0) * (temp_value-2000.0) / ((double)(1 << 4));
	sens_correction_value = 29.0 * (temp_value-2000.0) * (temp_value-2000.0) / ((double)(1 << 4));
      }
      temp_2nd_value -= temp_correction_value;
      off_2nd_value -= off_correction_value;
      sens_2nd_value -= sens_correction_value;

      temp_c_2nd_value = temp_2nd_value / 100.0;
      p_2nd_value = (d_value[0] *sens_2nd_value/((double)(1 << 21))-off_2nd_value)/((double)(1 << 15));
      pbar_2nd_value = p_2nd_value / 100.0;

      a_frame_info->press_meas_press = pbar_2nd_value;  
      a_frame_info->press_meas_temp = temp_c_2nd_value; 
      a_frame_info->rf_power_factor =  ((double)(a_frame_info->rf_power_meas[1])) / ((double)(a_frame_info->rf_power_meas[0]));
      a_frame_info->trolley_command_frame = &(a_frame_ushort[64]);

      if (a_frame_info->nmr_samples > 0) 
      {
	a_frame_info->nmr_waveform = &(a_frame_ushort[96]);           
      }
      else
      {
	a_frame_info->nmr_waveform = NULL;
      }

      if (a_frame_info->barcode_samples > 0) 
      {
	a_frame_info->barcode_waveform[0][0] = &(a_frame_ushort[96]) + a_frame_info->nmr_samples;           
	a_frame_info->barcode_waveform[0][1] = a_frame_info->barcode_waveform[0][0] + a_frame_info->barcode_samples; 
	a_frame_info->barcode_waveform[0][2] = a_frame_info->barcode_waveform[0][1] + a_frame_info->barcode_samples; 
	a_frame_info->barcode_waveform[1][0] = a_frame_info->barcode_waveform[0][1] + a_frame_info->barcode_samples; 
	a_frame_info->barcode_waveform[1][1] = a_frame_info->barcode_waveform[1][0] + a_frame_info->barcode_samples; 
	a_frame_info->barcode_waveform[1][2] = a_frame_info->barcode_waveform[1][1] + a_frame_info->barcode_samples; 
	a_frame_info->voltage_monitor_waveform[0] = a_frame_info->barcode_waveform[1][2] + a_frame_info->barcode_samples; 
	a_frame_info->voltage_monitor_waveform[1] = a_frame_info->voltage_monitor_waveform[0] + a_frame_info->barcode_samples;           
      }
      else
      {
	a_frame_info->barcode_waveform[0][0] = NULL;
	a_frame_info->barcode_waveform[0][1] = NULL;
	a_frame_info->barcode_waveform[0][2] = NULL;
	a_frame_info->barcode_waveform[1][0] = NULL; 
	a_frame_info->barcode_waveform[1][1] = NULL;
	a_frame_info->barcode_waveform[1][2] = NULL;
	a_frame_info->voltage_monitor_waveform[0] = NULL; 
	a_frame_info->voltage_monitor_waveform[1] = NULL; 
      }

      if (a_frame_info->flash_data_length > 0) 
      {
	a_frame_info->flash_data = &(a_frame_uchar[192]) + (a_frame_info->nmr_samples * sizeof(short int)) + 6 * (a_frame_info->barcode_samples * sizeof(unsigned short int));  
      }
      else
      {
	a_frame_info->flash_data = NULL;
      }
    }

    if (b_frame != NULL)
    {
      b_frame_info->mon_samples = b_frame_ushort[4];
      b_frame_info->power_status = b_frame_ushort[5];
      b_frame_info->ldo_status = b_frame_ushort[6];
      b_frame_info->reserved = b_frame_ushort[7];
      b_frame_info->timestamp_cycle_start_irig = b_frame_ulonglong[2];
      b_frame_info->timestamp_cycle_start_local = b_frame_ulonglong[3];
      b_frame_info->timestamp_monitor_start = b_frame_ulonglong[4];
      b_frame_info->temperature_monitor_min = (float)(b_frame_ushort[20]) * (25.0 * 5.0 / 65536.0);
      b_frame_info->temperature_monitor_max = (float)(b_frame_ushort[21]) * (25.0 * 5.0 / 65536.0);
      b_frame_info->voltage_monitor_min[0] = (float)(b_frame_ushort[22]) * (-1.25 * 5.0 / 65536.0);  // -5.0V    
      b_frame_info->voltage_monitor_max[0] = (float)(b_frame_ushort[23]) * (-1.25 * 5.0 / 65536.0);  // -5.0V    
      b_frame_info->voltage_monitor_min[1] = (float)(b_frame_ushort[24]) * (3.75 * 5.0 / 65536.0);  // 15.0V 
      b_frame_info->voltage_monitor_max[1] = (float)(b_frame_ushort[25]) * (3.75 * 5.0 / 65536.0);  // 15.0V 
      b_frame_info->voltage_monitor_min[2] = (float)(b_frame_ushort[26]) * (1.25 * 5.0 / 65536.0);  // 5.0V
      b_frame_info->voltage_monitor_max[2] = (float)(b_frame_ushort[27]) * (1.25 * 5.0 / 65536.0);  // 5.0V
      b_frame_info->voltage_monitor_min[3] = (float)(b_frame_ushort[28]) * (1.00 * 5.0 / 65536.0);  // 3.3V
      b_frame_info->voltage_monitor_max[3] = (float)(b_frame_ushort[29]) * (1.00 * 5.0 / 65536.0);  // 3.3V
      b_frame_info->rf_power_meas[0] = b_frame_ulong[15];
      b_frame_info->rf_power_meas[1] = b_frame_ulong[16];
      b_frame_info->reg_ti_switch_rf_offset = b_frame_ushort[34];
      b_frame_info->reg_ti_switch_comm_offset = b_frame_ushort[35];

      b_frame_info->rf_power_factor =  ((double)(b_frame_info->rf_power_meas[0])) / ((double)(b_frame_info->rf_power_meas[1]));

      if (b_frame_info->mon_samples > 0) 
      {
	b_frame_info->v_mon_waveform = &(b_frame_ushort[31]);           
	b_frame_info->i_mon_waveform = &(b_frame_ushort[31]) + b_frame_info->mon_samples;         
      }
      else
      {
	b_frame_info->v_mon_waveform = NULL;
	b_frame_info->i_mon_waveform = NULL;
      }
    }
    return;
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

  int DeviceLoadNMRSetting (NMR_Config NMR_Setting)
  {
    DeviceWrite(fifo_out_nmr_seq_data, NMR_Setting.NMR_Command);
    DeviceWrite(fifo_out_nmr_seq_data, NMR_Setting.NMR_Preamp_Delay);
    DeviceWrite(fifo_out_nmr_seq_data, NMR_Setting.NMR_Preamp_Period);
    DeviceWrite(fifo_out_nmr_seq_data, NMR_Setting.NMR_ADC_Gate_Delay);
    DeviceWrite(fifo_out_nmr_seq_data, NMR_Setting.NMR_ADC_Gate_Offset);
    DeviceWrite(fifo_out_nmr_seq_data, NMR_Setting.NMR_ADC_Gate_Period);
    DeviceWrite(fifo_out_nmr_seq_data, NMR_Setting.NMR_TX_Delay);
    DeviceWrite(fifo_out_nmr_seq_data, NMR_Setting.NMR_TX_Period);
    DeviceWrite(fifo_out_nmr_seq_data, NMR_Setting.User_Defined_Data);
    return 0;
  }

}//End namespace Trolley Interface

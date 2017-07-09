#include <iostream>
#include "TrolleyInterface.h"
#include "TFile.h"
#include "TGraph.h"
#include "TString.h"
#include <cstring>

#define TRLY_NMR_LENGTH 24000
#define TRLY_BARCODE_LENGTH 3000 //ALL CHANNELS
#define TRLY_BARCODE_CHANNELS 6
#define TRLY_MONITOR_LENGTH 3000

using namespace std;
using namespace TrolleyInterface;

int main(int argc,char **argv){
  int NReadings = 100;
  if (argc>1){
    NReadings = atoi(argv[1]);
  }


  int err = DeviceConnect("192.168.0.201");
  if (err<0)return -1;
  cout <<"connection good"<<endl;

  //init
  DeviceWrite(reg_power_control,0x00010000);

  //Disable Data flow
  DeviceWriteMask(reg_event_data_control,0x00000001,0x00000001);
  //Purge Data
  DevicePurgeData();

  //Configure Timing
  DeviceWrite(reg_timing_control,0x00003000);

  //Enable Data flow
  DeviceWriteMask(reg_event_data_control,0x00000001,0x00000000);

  DeviceWrite(reg_command,0x0000);


  //Receive Data
  char buffer[1000];
//  err = DataReceive(buffer);
//  cout <<err<<endl;

  //Connect to file
/*  const char * filename = "/home/newg2/Applications/field-daq/resources/NMRDataTemp/data-2017-02-28_20-08-11.dat"; 
  int err = FileOpen(filename);
*/
  //cout <<err<<endl;
  //Try to get some data

  int LastFrameNumber = 0;
  int FrameNumber = 0;
  int FrameSize = 0;
  //Frame buffer
  unsigned short* Frame = new unsigned short[MAX_PAYLOAD_DATA/sizeof(unsigned short)];
  unsigned short* FrameB = new unsigned short[MAX_PAYLOAD_DATA/sizeof(unsigned short)];
  unsigned int sizeA;
  unsigned int sizeB;

  //Read first frame and sync
  int rc = DataReceive((void *)Frame, (void *)FrameB, &sizeA, &sizeB);
  if (rc<0){
    cout <<"Data Error code "<<rc<<endl;
    return -1;
  }
  cout <<"first data"<<endl;
//  LastFrameNumber = *((int *)(&(Frame[9])));
  memcpy(&LastFrameNumber,&(Frame[9]),sizeof(int));
//  FrameSize = *((int *)(&(Frame[7])));
//  memcpy(&FrameSize,&(Frame[7]),sizeof(int));
//  cout <<"data read "<<rc<<", from Frame data read="<<FrameSize<<endl;
  cout << "A size = "<<sizeA<<" , B size = "<<sizeB<<endl;

  //Readout loop
  vector <int> zeros;
  int i=0;
  while (i<NReadings){
    //Read Frame
    rc = DataReceive((void *)Frame, (void *)FrameB, &sizeA, &sizeB);
    cout << "i= " << i <<" , A size = "<<sizeA<<" , B size = "<<sizeB<<endl;
    if (rc<0){
      cout <<"Data Error code "<<rc<<endl;
      return -1;
    }
  //  FrameNumber = *((int *)(&(Frame[9])));
    memcpy(&FrameNumber,&(Frame[9]),sizeof(int));
    //    FrameSize = *((int *)(&(Frame[7])));
    memcpy(&FrameSize,&(Frame[7]),sizeof(int));
/*    cout <<"Frame number = "<<FrameNumber<<" , Last "<<LastFrameNumber<<endl;
    cout <<"data read "<<rc<<", from Frame data read="<<FrameSize<<endl;
    cout<<"number of NMR samples "<<Frame[12]<<endl;
    cout<<"number of Barcode samples "<<Frame[13]<<endl;
*/
    LastFrameNumber=FrameNumber;

    i++;
  }

  //Disconnect
  DeviceWriteMask(0x40000944,0x00000001,0x00000001);
  err = DeviceDisconnect();

 // FileClose();
  //cout <<err<<endl;
  delete []Frame;
  delete []FrameB;
  return 0;
}

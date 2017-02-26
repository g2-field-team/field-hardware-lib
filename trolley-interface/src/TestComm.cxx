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

struct trolley_nmr_t{
  unsigned long int gps_clock;
  unsigned short probe_index;
  unsigned short length;
  short trace[TRLY_NMR_LENGTH];
};

struct trolley_barcode_t{
  unsigned long int gps_clock;
  unsigned short length_per_ch;
  unsigned short traces[TRLY_BARCODE_LENGTH]; //All channels
};

struct trolley_monitor_t{
  unsigned long int gps_clock_cycle_start;
  unsigned int PMonitorVal;
  unsigned int PMonitorTemp;
  unsigned int RFPower1;
  unsigned int RFPower2;
  unsigned short TMonitorIn;
  unsigned short TMonitorExt1;
  unsigned short TMonitorExt2;
  unsigned short TMonitorExt3;
  unsigned short V1Min;
  unsigned short V1Max;
  unsigned short V2Min;
  unsigned short V2Max;
  unsigned short length_per_ch;
  unsigned short trace_VMonitor1[TRLY_MONITOR_LENGTH];
  unsigned short trace_VMonitor2[TRLY_MONITOR_LENGTH];
};

bool BarcodeError;
bool PowersupplyStatus[3];
bool TemperatureInterupt;
unsigned short PressureSensorCal[7];
unsigned int NMRCheckSum;
unsigned int FrameCheckSum;
bool NMRCheckSumPassed;
bool FrameCheckSumPassed;


int main(int argc,char **argv){
  int NPulses = 10;
  int NBarcodes = 300;
  if (argc>1){
    NPulses = atoi(argv[1]);
  }
  if (argc>2){
    NBarcodes = atoi(argv[2]);
  }

/* 
  int err = DeviceConnect("192.168.1.123");
  if (err<0)return -1;
  cout <<"connection good"<<endl;
  //Read device
  unsigned int val;
  err = DeviceRead(0x40001060,&val);
  unsigned int LEDV = val & 0x03FF;
  unsigned int LEDDisable = val & 0x0400;
  cout <<err<<" "<<LEDV<< " "<<LEDDisable<<endl;
  DevicePurgeData();
  DeviceWriteMask(0x40000944,0x00000001,0x00000000);
*/
  //Receive Data
  char buffer[1000];
//  err = DataReceive(buffer);
//  cout <<err<<endl;

  //Connect to file
  const char * filename = "/home/newg2/Applications/field-daq/resources/NMRDataTemp/data_NMR_61682000Hz_11.70dbm-2016-10-27_19-36-42.dat"; 
  int err = FileOpen(filename);

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
  int rc = DataReceive((void *)Frame, (void *)FrameB, sizeA, sizeB);
  if (rc<0){
    cout <<"Data Error code "<<rc<<endl;
    return -1;
  }
  cout <<"first data"<<endl;
//  LastFrameNumber = *((int *)(&(Frame[9])));
  memcpy(&LastFrameNumber,&(Frame[9]),sizeof(int));
//  FrameSize = *((int *)(&(Frame[7])));
  memcpy(&FrameSize,&(Frame[7]),sizeof(int));
  cout <<"data read "<<rc<<", from Frame data read="<<FrameSize<<endl;
  cout<<"number of NMR samples "<<Frame[12]<<endl;

  TGraph **gArray = new TGraph*[NPulses];
  for (int j=0;j<NPulses;j++){
    gArray[j] = new TGraph();
    gArray[j]->SetName(Form("pulse%02d",j));
  }
  TGraph **gBarcodes = new TGraph*[6];
  for (int j=0;j<6;j++){
    gBarcodes[j] = new TGraph();
    gBarcodes[j]->SetName(Form("Barcode%d",j));
  }
  //Readout loop
  vector <int> zeros;
  int i=0;
  int gIndex = 0;
  int NCycle = NPulses;
  int iNMR=0;
  if (NBarcodes>NCycle)NCycle = NBarcodes;
  while (i<NCycle){
    //Read Frame
    rc = DataReceive((void *)Frame, (void *)FrameB, sizeA, sizeB);
    if (rc<0){
      cout <<"Data Error code "<<rc<<endl;
      return -1;
    }
 //   cout << i<<" "<<NCycle<<endl;
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

    trolley_nmr_t* TrlyNMRDataUnit = new trolley_nmr_t;
    trolley_barcode_t* TrlyBarcodeDataUnit = new trolley_barcode_t;
    trolley_monitor_t* TrlyMonitorDataUnit = new trolley_monitor_t;

    memcpy(&(TrlyNMRDataUnit->gps_clock),&(Frame[38]),sizeof(unsigned long int));
    TrlyNMRDataUnit->probe_index = (0x1F & Frame[11]);
    TrlyNMRDataUnit->length = Frame[12];
    unsigned short NSamNMR = Frame[12];
    //Check if this is larger than the MAX
    if (NSamNMR>TRLY_NMR_LENGTH){
      NSamNMR = TRLY_NMR_LENGTH;
    }
    for (unsigned short ii=0;ii<NSamNMR;ii++){
      TrlyNMRDataUnit->trace[ii] = (short)Frame[64+ii];
    }

    memcpy(&(TrlyBarcodeDataUnit->gps_clock),&(Frame[42]),sizeof(unsigned long int));
    TrlyBarcodeDataUnit->length_per_ch = Frame[13];
    unsigned short NSamBarcode = Frame[13]*TRLY_BARCODE_CHANNELS;
    //Check if this is larger than the MAX
    if (NSamBarcode>TRLY_BARCODE_LENGTH){
      NSamBarcode = TRLY_BARCODE_LENGTH;
    }
    for (unsigned short ii=0;ii<NSamBarcode;ii++){
      TrlyBarcodeDataUnit->traces[ii] = Frame[64+Frame[12]+ii];
    }
    memcpy(&(TrlyMonitorDataUnit->gps_clock_cycle_start),&(Frame[34]),sizeof(unsigned long int));
    memcpy(&(TrlyMonitorDataUnit->PMonitorVal),&(Frame[30]),sizeof(int));
    memcpy(&(TrlyMonitorDataUnit->PMonitorTemp),&(Frame[32]),sizeof(int));
    memcpy(&(TrlyMonitorDataUnit->RFPower1),&(Frame[60]),sizeof(int));
    memcpy(&(TrlyMonitorDataUnit->RFPower2),&(Frame[62]),sizeof(int));
    TrlyMonitorDataUnit->TMonitorIn = Frame[19];
    TrlyMonitorDataUnit->TMonitorExt1 = Frame[20];
    TrlyMonitorDataUnit->TMonitorExt2 = Frame[21];
    TrlyMonitorDataUnit->TMonitorExt3 = Frame[22];
    TrlyMonitorDataUnit->V1Min = Frame[15];
    TrlyMonitorDataUnit->V1Max = Frame[16];
    TrlyMonitorDataUnit->V2Min = Frame[17];
    TrlyMonitorDataUnit->V2Max = Frame[18];
    TrlyMonitorDataUnit->length_per_ch = Frame[13];
    for (unsigned short ii=0;ii<Frame[13];ii++){
      TrlyMonitorDataUnit->trace_VMonitor1[ii] = Frame[64+Frame[12]+Frame[13]*TRLY_BARCODE_CHANNELS+ii];
    }
    for (unsigned short ii=0;ii<Frame[13];ii++){
      TrlyMonitorDataUnit->trace_VMonitor1[ii] = Frame[64+NSamNMR+NSamBarcode+Frame[13]+ii];
    }
    BarcodeError = bool(0x100 & Frame[11]);
    TemperatureInterupt = bool(0x200 & Frame[11]);
    PowersupplyStatus[0] = bool(0x400 & Frame[11]);
    PowersupplyStatus[1] = bool(0x800 & Frame[11]);
    PowersupplyStatus[2] = bool(0x1000 & Frame[11]);
    memcpy(&(NMRCheckSum),&(Frame[46]),sizeof(int));
    memcpy(&(FrameCheckSum),&(Frame[64+NSamNMR+NSamBarcode+Frame[13]*2]),sizeof(int));
    for (short ii=0;ii<7;ii++){
      PressureSensorCal[ii] = Frame[23+ii];;
    }

    //Fill Graph
    if (iNMR<NPulses && Frame[12]>0){
      zeros.clear();
      for (int j=0;j<TrlyNMRDataUnit->length;j++){
	gArray[iNMR]->SetPoint(j,j,TrlyNMRDataUnit->trace[j]);
	if (TrlyNMRDataUnit->trace[j-1]*TrlyNMRDataUnit->trace[j]<=0 && j>4000 && j<14400){
	  zeros.push_back(j);
	}
      }
      int n = zeros.size();
      double freq = (1.0/(2.0*double(zeros[n-1]*0.001 - zeros[0]*0.001)/double(zeros.size()-1)));
      //      long double freq = (1000.0/(2.0*(long double)(zeros[n-1]*0.001 - zeros[0]*0.001)/(long double)(zeros.size()-1))) - 50394.7;
      cout <<"Frequency = "<< freq<<endl;
      iNMR++;
    }
    if (i<NBarcodes){
      for (short ii=0;ii<Frame[13];ii++){
	double BTime = TrlyBarcodeDataUnit->gps_clock*50/1000000.0+0.1*ii;
	for (short jj=0;jj<6;jj++){
	  gBarcodes[jj]->SetPoint(gIndex,BTime,TrlyBarcodeDataUnit->traces[jj*Frame[13]+ii]);
	}
	gIndex++;
      }
    }

    delete TrlyNMRDataUnit;
    delete TrlyBarcodeDataUnit;
    delete TrlyMonitorDataUnit;
    i++;
  }

  TFile f("Testout.root","recreate");
  for (int j=0;j<NPulses;j++){
    gArray[j]->Write();
  }
  for (int j=0;j<6;j++){
    gBarcodes[j]->Write();
  }
  f.Close();
  //Test file operations
  while(1){
    int rc = DataReceive((void *)Frame, (void *)FrameB, sizeA, sizeB);
    if (rc<0){
      cout <<"Data Error code "<<rc<<endl;
      return -1;
    }
  }

  //Disconnect
/*  DeviceWriteMask(0x40000944,0x00000001,0x00000000);
  err = DeviceDisconnect();
*/
  FileClose();
  //cout <<err<<endl;
  delete []Frame;
  for (int j=0;j<NPulses;j++){
    delete gArray[j];
  }
  delete []gArray;
  for (int j=0;j<6;j++){
    delete gBarcodes[j];
  }
  delete []gBarcodes;
  return 0;
}

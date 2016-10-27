#include <iostream>
#include "TrolleyInterface.h"
#include "TFile.h"
#include "TGraph.h"
#include "TString.h"
#include <cstring>

using namespace std;
using namespace TrolleyInterface;

typedef struct _TrlyDataStruct{
  //NMR
  unsigned long int NMRTimeStamp;
  unsigned short ProbeNumber;
  unsigned short NSample_NMR;
  short NMRSamples[MAX_NMR_SAMPLES];

  //Barcode
  unsigned long int BarcodeTimeStamp;
  unsigned short NSample_Barcode_PerCh;
  short BarcodeSamples[MAX_BARCODE_SAMPLES];

  //Monitor
  bool BarcodeError;
  short BCToNMROffset;
  short VMonitor1;
  short VMonitor2;
  unsigned short TMonitorIn;
  unsigned short TMonitorExt1;
  unsigned short TMonitorExt2;
  unsigned short TMonitorExt3;
  unsigned short PressureSensorCal[7];
  unsigned int PMonitorVal;
  unsigned int PMonitorTemp;

  //Register Readbacks
  unsigned short BarcodeRegisters[5];
  unsigned short TrlyRegisters[16];

  //Constructor
  _TrlyDataStruct(){}
  _TrlyDataStruct(const _TrlyDataStruct& obj)
  {
    //NMR
    NMRTimeStamp = obj.NMRTimeStamp;
    ProbeNumber = obj.ProbeNumber;
    NSample_NMR = obj.NSample_NMR;
    for (int i=0;i<MAX_NMR_SAMPLES;i++){
      NMRSamples[i] = obj.NMRSamples[i];
    }

    //Barcode
    BarcodeTimeStamp = obj.BarcodeTimeStamp;
    NSample_Barcode_PerCh = obj.NSample_Barcode_PerCh;
    for (int i=0;i<MAX_BARCODE_SAMPLES;i++){
      BarcodeSamples[i] = obj.BarcodeSamples[i];
    }
    //Monitor
    BarcodeError = obj.BarcodeError;
    BCToNMROffset = obj.BCToNMROffset;
    VMonitor1 = obj.VMonitor1;
    VMonitor2 = obj.VMonitor2;
    TMonitorIn = obj.TMonitorIn;
    TMonitorExt1 = obj.TMonitorExt1;
    TMonitorExt2 = obj.TMonitorExt2;
    TMonitorExt3 = obj.TMonitorExt3;
    for(int i=0;i<7;i++){
      PressureSensorCal[i] = obj.PressureSensorCal[i];
    }
    PMonitorVal = obj.PMonitorVal;
    PMonitorTemp = obj.PMonitorTemp;
    for(int i=0;i<5;i++){
      BarcodeRegisters[i] = obj.BarcodeRegisters[i];
    }
    for(int i=0;i<16;i++){
      TrlyRegisters[i] = obj.TrlyRegisters[i];
    }
  }
}TrlyDataStruct;


int main(int argc,char **argv){
  int NPulses = 10;
  int NBarcodes = 300;
  if (argc>1){
    NPulses = atoi(argv[1]);
  }
  if (argc>2){
    NBarcodes = atoi(argv[2]);
  }
  int err = DeviceConnect("192.168.1.123");
  cout <<err<<endl;
  if (err<0)return -1;
  //Read device
  unsigned int val;
  err = DeviceRead(0x40001060,&val);
  unsigned int LEDV = val & 0x03FF;
  unsigned int LEDDisable = val & 0x0400;
  cout <<err<<" "<<LEDV<< " "<<LEDDisable<<endl;
  //Receive Data
  char buffer[1000];
//  err = DataReceive(buffer);
//  cout <<err<<endl;

  //Try to get some data

  int LastFrameNumber = 0;
  int FrameNumber = 0;
  int FrameSize = 0;
  //Frame buffer
  short* Frame = new short[MAX_PAYLOAD_DATA/sizeof(short)];

  //Read first frame and sync
  int rc = DataReceive((void *)Frame);
  if (rc<0){
    cout <<"Data Error code "<<rc<<endl;
    return -1;
  }
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
  int i=0;
  int gIndex = 0;
  int NCycle = NPulses;
  if (NBarcodes>NCycle)NCycle = NBarcodes;
  while (i<NCycle){
    //Read Frame
    rc = DataReceive((void *)Frame);
    if (rc<0){
      cout <<"Data Error code "<<rc<<endl;
      return -1;
    }
  //  FrameNumber = *((int *)(&(Frame[9])));
    memcpy(&FrameNumber,&(Frame[9]),sizeof(int));
    //    FrameSize = *((int *)(&(Frame[7])));
    memcpy(&FrameSize,&(Frame[7]),sizeof(int));
//    cout <<"Frame number = "<<FrameNumber<<" , Last "<<LastFrameNumber<<endl;
//    cout <<"data read "<<rc<<", from Frame data read="<<FrameSize<<endl;

    LastFrameNumber=FrameNumber;
    //Translate buffer into TrlyDataStruct
    TrlyDataStruct* TrlyDataUnit = new TrlyDataStruct;

//    TrlyDataUnit->NMRTimeStamp = *((long int *)(&(Frame[32])));
    memcpy(&(TrlyDataUnit->NMRTimeStamp),&(Frame[32]),sizeof(unsigned long int));
    TrlyDataUnit->ProbeNumber = short(0x1F & Frame[11]);
    TrlyDataUnit->NSample_NMR = Frame[12];
    short NSamNMR = Frame[12];
/*    cout<<"NMR TimeStamp "<<TrlyDataUnit->NMRTimeStamp<<" ; ";
    cout<<"number of NMR samples "<<TrlyDataUnit->NSample_NMR<<endl;
*/
    for (short ii=0;ii<NSamNMR;ii++){
      TrlyDataUnit->NMRSamples[ii] = Frame[64+ii];
    }
    //TrlyDataUnit->BarcodeTimeStamp = *((long int *)(&(Frame[36]))) ;
    memcpy(&(TrlyDataUnit->BarcodeTimeStamp),&(Frame[36]),sizeof(unsigned long int));
    TrlyDataUnit->NSample_Barcode_PerCh = Frame[13];
    short NSamBarcode = Frame[13]*BARCODE_CH_NUM;
/*    cout<<"Barcode TimeStamp "<<TrlyDataUnit->BarcodeTimeStamp<<" ; ";
    cout<<"number of Barcode Channels "<<BARCODE_CH_NUM<<" ; ";
    cout<<"number of Barcode samples per Ch "<<TrlyDataUnit->NSample_Barcode_PerCh<<endl;
*/
    //Check if this is larger than the MAX
    for (short ii=0;ii<NSamBarcode;ii++){
      TrlyDataUnit->BarcodeSamples[ii] = Frame[64+NSamNMR+ii];
    }
    TrlyDataUnit->BarcodeError = bool(0x100 & Frame[11]);
    TrlyDataUnit->BCToNMROffset = Frame[14];
    TrlyDataUnit->VMonitor1 = Frame[15];
    TrlyDataUnit->VMonitor2 = Frame[16];
    TrlyDataUnit->TMonitorIn = Frame[17];
    TrlyDataUnit->TMonitorExt1 = Frame[18];
    TrlyDataUnit->TMonitorExt2 = Frame[19];
    TrlyDataUnit->TMonitorExt3 = Frame[20];
    if (i==0){
      cout <<"Offset " <<TrlyDataUnit->BCToNMROffset<<" "<<(int(TrlyDataUnit->BarcodeTimeStamp)-int(TrlyDataUnit->NMRTimeStamp))<<endl;
      cout <<"VMonitor " <<TrlyDataUnit->VMonitor1<<" "<<TrlyDataUnit->VMonitor2<<endl;
      cout <<"TMonitors " <<TrlyDataUnit->TMonitorExt1<<" "<<TrlyDataUnit->TMonitorExt2<<endl;
      cout <<Frame[43]<<endl;
    }
    for (short ii=0;ii<7;ii++){
      TrlyDataUnit->PressureSensorCal[ii] = Frame[21+ii];;
    }
//    TrlyDataUnit->PMonitorVal = *((int *)(&(Frame[28]))) ;
//    TrlyDataUnit->PMonitorTemp = *((int *)(&(Frame[30])));
    memcpy(&(TrlyDataUnit->PMonitorVal),&(Frame[28]),sizeof(int));
    memcpy(&(TrlyDataUnit->PMonitorTemp),&(Frame[30]),sizeof(int));
    for (short ii=0;ii<5;ii++){
      TrlyDataUnit->BarcodeRegisters[ii] = Frame[43+ii];
    }
    for (short ii=0;ii<16;ii++){
      TrlyDataUnit->TrlyRegisters[ii] = Frame[48+ii];
    }
    //Fill Graph
    if (i<NPulses){
      for (int j=0;j<TrlyDataUnit->NSample_NMR;j++){
	gArray[i]->SetPoint(j,j,TrlyDataUnit->NMRSamples[j]);
      }
    }
    if (i<NBarcodes){
      for (short ii=0;ii<Frame[13];ii++){
	double BTime = TrlyDataUnit->BarcodeTimeStamp*50/1000000.0+0.1*ii;
	for (short jj=0;jj<6;jj++){
	  gBarcodes[jj]->SetPoint(gIndex,BTime,TrlyDataUnit->BarcodeSamples[jj*Frame[13]+ii]);
	}
	gIndex++;
      }
    }

    delete TrlyDataUnit;
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

  //Disconnect
  err = DeviceDisconnect();
  cout <<err<<endl;
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

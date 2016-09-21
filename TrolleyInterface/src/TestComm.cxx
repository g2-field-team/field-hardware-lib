#include <iostream>
#include "TrolleyInterface.h"
#include "TFile.h"
#include "TGraph.h"

using namespace std;
using namespace TrolleyInterface;

int main(){
  int err = DeviceConnect("192.168.1.123");
  cout <<err<<endl;
  //Read device
  unsigned int val;
  err = DeviceRead(0x40003400,&val);
  cout <<err<<" "<<val<<endl;
  //Receive Data
  char buffer[1000];
//  err = DataReceive(buffer);
//  cout <<err<<endl;
  //Try to get some data
  short * data;
  data = new short[100000];
  TGraph *g=new TGraph();

  int rc = DataReceive((void *)data);
  cout <<"data read "<<rc<<", number of samples "<<data[12]<<endl;
  for (int i=0;i<data[12];i++){
    g->SetPoint(i,i,data[64+i]);
  }
  TFile f("Testout.root","recreate");
  g->SetName("TestGraph");
  g->Write();
  f.Close();

  //Disconnect
  err = DeviceDisconnect();
  cout <<err<<endl;
  delete []data;
  delete g;
  return 0;
}

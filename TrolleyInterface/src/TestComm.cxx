#include <iostream>
#include "Device.h"

using namespace std;
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
  err = DeviceDisconnect();
  cout <<err<<endl;
  return 0;
}

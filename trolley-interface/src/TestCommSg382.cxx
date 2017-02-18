#include <iostream>
#include "Sg382Interface.h"
#include "TCPDrivers.h"
#include <cstring>

using namespace std;
using namespace TCPDrivers;
using namespace Sg382Interface;

int main(int argc,char **argv){
  int error = 0;
  double f = 0;
  double A = 0;
  int Status = -1;;
  error = Sg382Connect("192.168.1.122");

  GetFrequency(f);
  GetAmplitude(A);
  GetStatus(Status);
      
  cout <<"Frequency "<<f<<endl;
  cout <<"Amplitude "<<A<<endl;
  cout <<"Status "<<Status<<endl;

  SetFrequency(61.766);
  SetAmplitude(6.1);
  DisableRF();
  GetFrequency(f);
  GetAmplitude(A);
  GetStatus(Status);
      
  cout <<"Frequency "<<f<<endl;
  cout <<"Amplitude "<<A<<endl;
  cout <<"Status "<<Status<<endl;

  EnableRF();
  GetStatus(Status);
  cout <<"Status "<<Status<<endl;

  error = Sg382Disconnect();
  return 0;
}

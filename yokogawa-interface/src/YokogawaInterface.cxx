#include <stdio.h>

#include "YokogawaInterface.hh"
#include "vxi11_user.h"

namespace yokogawa_interface { 
   CLINK *clink = NULL;                                     // for VXI-11 connection  
   //___________________________________________________________________________
   int set_mode(int mode){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      switch(mode){
	 case kVOLTAGE: 
	    sprintf(cmd,":SOUR:FUNC VOLT");
	    break;
	 case kCURRENT: 
	    sprintf(cmd,":SOUR:FUNC CURR");
	    break;
	 default:
	    // invalid mode, return with error  
	    return -1;
      }
      int rc = write(cmd); 
      delete cmd;
      return rc; 
   }
   //___________________________________________________________________________
   int set_range(double r){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:RANG %.5E",r);
      int rc = write(cmd); 
      delete cmd;
      return rc; 
   }
   //___________________________________________________________________________
   int set_range_min(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:RANG MIN");
      int rc = write(cmd); 
      delete cmd;
      return rc; 
   }
   //___________________________________________________________________________
   int set_range_max(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:RANG MAX");
      int rc = write(cmd); 
      delete cmd;
      return rc; 
   }
   //___________________________________________________________________________
   int set_level(double lvl){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:LEV %.5E",lvl);
      int rc = write(cmd); 
      delete cmd;
      return rc; 
   }
   //___________________________________________________________________________
   int set_output_state(int state){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":OUTP:STAT %d",state);
      int rc = write(cmd); 
      delete cmd;
      return rc; 
   }
   //___________________________________________________________________________
   int set_clock_time(int hr,int min,int sec){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SYST:CLOC:TIME %2d:%2d:%2d",hr,min,sec);
      int rc = write(cmd); 
      delete cmd;
      return rc; 
   }
   //___________________________________________________________________________
   int set_clock_date(int day,int month,int year){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SYST:CLOC:DATE %4d/%2d/%2d",year,month,day);
      int rc = write(cmd); 
      delete cmd; 
      return rc;
   }
   //___________________________________________________________________________
   int self_test(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,"*TST?");
      std::string response = ask(cmd);
      int rc = atoi( response.c_str() );
      delete cmd; 
      return rc;  
   }  
   //___________________________________________________________________________
   int error_check(char *err_msg){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SYSTem::ERRor?");
      std::string response = ask(cmd);
      // parse the string; it's going to be an error code and a message
      std::istringstream ss(response);
      std::string token,entry[2];
      int i=0;
      while( std::getline(ss,token,',') ){
         entry[i] = token;
         i++;
      }
      // now return the data 
      int rc = atoi(entry[0].c_str());     // zeroth entry is the error code
      strcpy( err_msg,entry[1].c_str() );  // copy the message to the buffer 
      delete cmd; 
      return rc;  
   }  
   //___________________________________________________________________________
   int clear_errors(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,"*CLS");
      int rc = write(cmd);
      delete cmd; 
      return rc;  
   }
   //___________________________________________________________________________
   int get_output_state(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,"OUTP?");
      std::string response = ask(cmd);
      int rc = atoi( response.c_str() );
      delete cmd; 
      return rc;  
   }  
   //___________________________________________________________________________
   int get_mode(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:FUNC?");
      std::string response = ask(cmd);
      int rc=-1; 
      int res = response.compare(0,4,"CURR"); 

      if (res==0) { 
	 rc = kCURRENT;
      } else {
         rc = kVOLTAGE; 
      }
      delete cmd; 
      return rc;  
   }
   //___________________________________________________________________________
   double get_level(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:LEV?");
      std::string response = ask(cmd);
      double lvl = atof( response.c_str() );
      delete cmd; 
      return lvl;  
   }  
   //___________________________________________________________________________
   double get_range(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:RANG?");
      std::string response = ask(cmd);
      double r = atof( response.c_str() );
      delete cmd; 
      return r;  
   }
   //___________________________________________________________________________
   std::string get_device_id(){
      const int SIZE = 100; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,"*IDN?");
      std::string response = ask(cmd);
      delete cmd; 
      return response; 
   }  
   //___________________________________________________________________________
   std::string get_clock_time(){
      const int SIZE = 100; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SYST:CLOC:TIME?");
      std::string response = ask(cmd);
      delete cmd; 
      return response; 
   } 
   //___________________________________________________________________________
   std::string get_clock_date(){
      const int SIZE = 100; 
      char *cmd = new char[SIZE+1]; 
      std::string response;  
      sprintf(cmd,":SYST:CLOC:DATE?");
      response = ask(cmd);
      delete cmd; 
      return response; 
   } 
   //___________________________________________________________________________
   int write(const char *cmd){
      int rc = vxi11_send(clink,cmd); 
      return rc;
   } 
   //___________________________________________________________________________
   std::string ask(const char *cmd){
      const int SIZE = 512; 
      char theBuf[SIZE];  
      int rc = vxi11_send_and_receive(clink,cmd,theBuf,SIZE,100);  // last argument is a timeout 
      if(rc!=0) strcpy(theBuf,"NO RESPONSE");                      // comms failed   
      std::string response = std::string( theBuf ); 
      return response;
   }
   //___________________________________________________________________________
   int open_connection(const char *ip_addr){
      sprintf(DeviceIP,ip_addr); 
      if (clink==NULL)clink = new CLINK;
      printf("Attempting to open the connection to IP address %s... \n",ip_addr); 
      int rc = vxi11_open_device(DeviceIP,clink);
      printf("Returning... \n"); 
      return rc;  
   }
   //___________________________________________________________________________
   int close_connection(){
      int rc = vxi11_close_device(DeviceIP,clink);
      if (clink!=NULL) delete clink;
      clink = NULL;
      return rc;
   }
}

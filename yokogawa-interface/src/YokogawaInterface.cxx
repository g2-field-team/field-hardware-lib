#include "YokogawaInterface.hh"

namespace yokogawa_interface { 

   //___________________________________________________________________________
   void set_mode(int mode){
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
	    sprintf(cmd,":SOUR:FUNC CURR");
      }
      write(cmd); 
      delete cmd; 
   }
   //___________________________________________________________________________
   void set_range(double r){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:RANG %.5E",r);
      write(cmd); 
      delete cmd; 
   }
   //___________________________________________________________________________
   void set_range_min(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:RANG MIN");
      write(cmd); 
      delete cmd; 
   }
   //___________________________________________________________________________
   void set_range_max(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:RANG MAX");
      write(cmd); 
      delete cmd; 
   }
   //___________________________________________________________________________
   void set_level(double lvl){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:LEV %.5lf",lvl);
      write(cmd); 
      delete cmd; 
   }
   //___________________________________________________________________________
   void set_output_state(int state){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":OUTP:STAT %d",state);
      write(cmd); 
      delete cmd; 
   }
   //___________________________________________________________________________
   void set_clock_time(int hr,int min,int sec){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SYST:CLOC:TIME %2d:%2d:%2d",hr,min,sec);
      write(cmd); 
      delete cmd; 
   }
   //___________________________________________________________________________
   void set_clock_date(int day,int month,int year){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SYST:CLOC:DATE %4d/%2d/%2d",year,month,day);
      write(cmd); 
      delete cmd; 
   }
   //___________________________________________________________________________
   void write(const char *cmd){
      // FIXME: does nothing yet
   } 
   //___________________________________________________________________________
   int open_vxi11_connection(const char *ip_addr){
      sprintf(DeviceIP,ip_addr); 
      const int SIZE = 100; 
      char *visa_str = new char[SIZE+1]; 
      sprintf(visa_str,"TCPIP::%s::INSTR",DeviceIP);
      // FIXME: open the connection 
      delete visa_str;
      return 0;  
   }
   //___________________________________________________________________________
   int close_vxi11_connection(const char *ip_addr){
      const int SIZE = 100; 
      char *visa_str = new char[SIZE+1]; 
      sprintf(visa_str,"TCPIP::%s::INSTR",DeviceIP);
      // FIXME: close the connection 
      delete visa_str;
      return 0;  
   }
   //___________________________________________________________________________
   int self_test(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,"*TST?");
      char *ans = ask(cmd);
      int rc = atoi(ans);
      delete cmd; 
      return rc;  
   }  
   //___________________________________________________________________________
   int error_check(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SYSTem::ERRor?");
      char *ans = ask(cmd);
      int rc = atoi(ans);
      delete cmd; 
      return rc;  
   }  
   //___________________________________________________________________________
   int get_output_state(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,"OUTP?");
      char *ans = ask(cmd);
      int rc = atoi(ans);
      delete cmd; 
      return rc;  
   }  
   //___________________________________________________________________________
   int get_mode(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:FUNC?");
      char *ans = ask(cmd);
      int rc=-1; 
      int res = strcmp(ans,"CURR"); 
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
      char *ans = ask(cmd);
      double lvl = atof(ans);
      delete cmd; 
      return lvl;  
   }  
   //___________________________________________________________________________
   double get_range(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:RANG?");
      char *ans = ask(cmd);
      double r = atof(ans);
      delete cmd; 
      return r;  
   }
   //___________________________________________________________________________
   char *ask(const char *cmd){
      // FIXME: need to add this 
      char *res = "NO RESPONSE"; 
      return res;  
   }
   //___________________________________________________________________________
   char *get_device_id(){
      const int SIZE = 100; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,"*IDN?");
      char *ans = ask(cmd);
      delete cmd; 
      return ans; 
   }  
   //___________________________________________________________________________
   char *get_clock_time(){
      const int SIZE = 100; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SYST:CLOC:TIME?");
      char *ans = ask(cmd);
      delete cmd; 
      return ans; 
   } 
   //___________________________________________________________________________
   char *get_clock_date(){
      const int SIZE = 100; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SYST:CLOC:DATE?");
      char *ans = ask(cmd);
      delete cmd; 
      return ans; 
   } 


}

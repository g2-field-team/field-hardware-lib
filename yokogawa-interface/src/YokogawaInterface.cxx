#include "YokogawaInterface.hh"

namespace yokogawa_interface { 
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
      sprintf(cmd,":SOUR:LEV %.5lf",lvl);
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
      buf = ask(cmd);
      int rc = atoi(buf);
      delete cmd; 
      return rc;  
   }  
   //___________________________________________________________________________
   int error_check(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SYSTem::ERRor?");
      buf    = ask(cmd);
      int rc = atoi(buf);
      delete cmd; 
      return rc;  
   }  
   //___________________________________________________________________________
   int get_output_state(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,"OUTP?");
      buf    = ask(cmd);
      int rc = atoi(buf);
      delete cmd; 
      return rc;  
   }  
   //___________________________________________________________________________
   int get_mode(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:FUNC?");
      buf = ask(cmd);
      int rc=-1; 
      int res = strcmp(buf,"CURR"); 
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
      buf = ask(cmd);
      double lvl = atof(buf);
      delete cmd; 
      return lvl;  
   }  
   //___________________________________________________________________________
   double get_range(){
      const int SIZE = 20; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SOUR:RANG?");
      buf = ask(cmd);
      double r = atof(buf);
      delete cmd; 
      return r;  
   }
   //___________________________________________________________________________
   char *get_device_id(){
      const int SIZE = 100; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,"*IDN?");
      buf = ask(cmd);
      delete cmd; 
      return buf; 
   }  
   //___________________________________________________________________________
   char *get_clock_time(){
      const int SIZE = 100; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SYST:CLOC:TIME?");
      buf = ask(cmd);
      delete cmd; 
      return buf; 
   } 
   //___________________________________________________________________________
   char *get_clock_date(){
      const int SIZE = 100; 
      char *cmd = new char[SIZE+1]; 
      sprintf(cmd,":SYST:CLOC:DATE?");
      buf = ask(cmd);
      delete cmd; 
      return buf; 
   } 
   //___________________________________________________________________________
   int write(const char *cmd){
      int rc = vxi11_send(clink,cmd); 
      return rc;
   } 
   //___________________________________________________________________________
   char *ask(const char *cmd){
      int rc = write(cmd); 
      int bytes_rec=0;
      const int SIZE = 100; 
      strcpy(REC_BUF,"");   // clear the receive buffer   
      if(rc!=0){
	 // error!
	 sprintf(REC_BUF,"ERROR, can't send query: %s \n",cmd);  
      }else{
	 bytes_rec = vxi11_receive(clink,REC_BUF,SIZE);
         if(bytes_rec<=0){
	    sprintf(REC_BUF,"ERROR, can't read result of query: %s \n",cmd);  
         }
      }
      return REC_BUF;  
   }
   //___________________________________________________________________________
   int open_connection(const char *ip_addr){
      buf     = new char[YOKO_BUF_SIZE+1]; 
      REC_BUF = new char[YOKO_BUF_SIZE+1]; 
      sprintf(DeviceIP,ip_addr); 
      int rc = vxi11_open_device(DeviceIP,clink,DeviceName);
      return rc;  
   }
   //___________________________________________________________________________
   int close_connection(){
      int rc = vxi11_close_device(DeviceIP,clink);
      delete buf;
      delete REC_BUF; 
      return rc;  
   }
}

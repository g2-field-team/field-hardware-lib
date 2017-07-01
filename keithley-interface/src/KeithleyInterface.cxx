#include "KeithleyInterface.hh"

namespace keithley_interface {
   //______________________________________________________________________________
   int open_connection(int type,const char *dev_name,const char *dev_path){
      int handle = CommDriver::open_connection(type,dev_name,dev_path);
      return handle;
   }
   //______________________________________________________________________________
   int close_connection(int type,int handle){
      int rc = CommDriver::close_connection(type,handle);
      return rc;
   }
   //______________________________________________________________________________
   int get_device_id(int type,int portNo,char *response){
      const int SIZE = 512;
      char cmd[SIZE];
      sprintf(cmd,"*IDN?\n");
      int rc = CommDriver::query(type,portNo,cmd,response);
      return rc;
   }
   //______________________________________________________________________________
   int get_mode(int type,int portNo,char *response){
      const int SIZE = 512;
      char cmd[SIZE];
      sprintf(cmd,"SENS:FUNC?\n");
      int rc = CommDriver::query(type,portNo,cmd,response);
      return rc;
   }
   //______________________________________________________________________________
   int check_errors(int type,int portNo,char *err_msg){
      const int SIZE = 512;
      char cmd[SIZE],response[512];
      sprintf(cmd,"SYST:ERR?\n");
      int rc = CommDriver::query(type,portNo,cmd,response);
      // parse the string; it's going to be an error code and a message
      std::istringstream ss(response);
      std::string token,entry[2];
      int i=0;
      while( std::getline(ss,token,',') ){
         entry[i] = token;
         i++;
      }
      // now return the data 
      rc = atoi(entry[0].c_str());     // zeroth entry is the error code
      strcpy( err_msg,entry[1].c_str() );  // copy the message to the buffer 
      return rc;
   }
   //___________________________________________________________________________
   int clear_errors(int type,int portNo){
      char cmd[20];
      sprintf(cmd,"*CLS");
      int rc = CommDriver::write_cmd(type,portNo,cmd);
      return rc;
   }
   //______________________________________________________________________________
   int set_to_remote_mode(int type,int portNo){
      const int SIZE = 512;
      char cmd[SIZE];
      sprintf(cmd,"SYST:REM\n");
      int rc   = CommDriver::write_cmd(type,portNo,cmd);
      return rc;
   }
   //______________________________________________________________________________
   int set_range(int type,int portNo,double maxRange){
      const int SIZE = 512;
      char cmd[SIZE];
      sprintf(cmd,"CONF:RES %.3lf\n",maxRange);
      int rc   = CommDriver::write_cmd(type,portNo,cmd);
      return rc;
   }
   //______________________________________________________________________________
   int get_resistance(int type,int portNo,double &R){
      const int SIZE = 512;
      char query[SIZE],response[SIZE];
      sprintf(query,"MEAS:RES?\n");
      int rc = CommDriver::query(type,portNo,query,response);
      R      = atof(response);
      return rc;
   }
}  // ::keithley_interface 

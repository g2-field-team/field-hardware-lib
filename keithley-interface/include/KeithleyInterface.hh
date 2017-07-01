#ifndef KEITHLEY_INTERFACE_HH
#define KEITHLEY_INTERFACE_HH

// functions for comms with the Keithley DMM

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <fcntl.h>    // For O_RDWR 
#include <unistd.h>   // For open(), creat() 

#include "CommDriver.hh"

namespace keithley_interface {

   int open_connection(int type,const char *dev_name,const char *dev_path);
   int close_connection(int type,int portNo);
   int check_errors(int type,int portNo,char *err_msg);
   int clear_errors(int type,int portNo);

   int set_to_remote_mode(int type,int portNo);
   int set_range(int type,int portNo,double maxRange);

   int get_device_id(int type,int portNo,char *response);
   int get_mode(int type,int portNo,char *response);
   int get_resistance(int type,int portNo,double &R);

}

#endif

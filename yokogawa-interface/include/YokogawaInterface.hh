#ifndef YOKOGAWAINTERFACE_HH 
#define YOKOGAWAINTERFACE_HH

//==============================================================================
//
// Title:       YokogawaInterface.hh 
// Description: Library to talk to the Yokogawa current source 
// Author:      David Flay (flay@umass.edu) 
//
//==============================================================================

#include <cstdlib> 
#include <cstdio>
#include <vector>  
#include <iostream>
#include <sstream> 
#include <string.h> 

//#include "vxi11/vxi11_user.h"
//#include "vxi11/vxi11_xdr.c"
//#include "vxi11/vxi11_clnt.c"

#define YOKO_BUF_SIZE 1000 

namespace yokogawa_interface { 

   enum yokoMode { 
      kVOLTAGE = 0, 
      kCURRENT = 1
   };

   enum yokoState { 
      kDISABLED = 0, 
      kENABLED  = 1
   }; 
   
   //CLINK *clink;                                     // for VXI-11 connection  

   // char *buf;                                        // for queries (in other functions)   
   // char *REC_BUF;                                    // for queries (in ask function; this gets returned to buf)  
   char DeviceName[20]  = "Yokogawa_GS210";
   char DeviceIP[20]    = "XXX.XXX.XXX.XXX";
   char MfgName[50]     = "UNKNOWN";
   char ModelNo[50]     = "UNKNOWN";
   char SerialNo[50]    = "UNKNOWN";
   char FirmwareVer[50] = "UNKNOWN";
 
   int set_mode(int mode); 
   int set_range(double r); 
   int set_range_min(); 
   int set_range_max(); 
   int set_level(double lvl); 
   int set_output_state(int state); 
   int set_clock_time(int hr,int min,int sec);  
   int set_clock_date(int month,int day,int year); 
  
   int clear_errors(); 
   int write(const char *); 
   int open_connection(const char *ip_addr); 
   int close_connection(); 

   int self_test(); 
   int error_check(char *err_msg); 

   int get_output_state(); 
   int get_mode(); 
 
   double get_level(); 
   double get_range();
 
   std::string ask(const char *); 
   std::string get_device_id(); 
   std::string get_clock_time(); 
   std::string get_clock_date(); 

}

#endif  

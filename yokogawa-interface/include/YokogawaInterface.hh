#ifndef YOKOGAWAINTERFACE_HH 
#define YOKOGAWAINTERFACE_HH

//==============================================================================
//
// Title:       YokogawaInterface.hh 
// Description: Library to talk to the Yokogawa current source 
// Author:      David Flay (flay@umass.edu) 
//
//==============================================================================

#include <iostream>
#include <cstdlib> 
#include <cstdio> 
#include <string.h> 

namespace yokogawa_interface { 

   enum yokoMode { 
      kVOLTAGE = 0, 
      kCURRENT = 1
   };

   enum yokoState { 
      kOFF = 0, 
      kON  = 1
   }; 

   char DeviceIP[20]    = "XXX.XXX.XXX.XXX";
   char MfgName[50]     = "UNKNOWN";
   char ModelNo[50]     = "UNKNOWN";
   char SerialNo[50]    = "UNKNOWN";
   char FirmwareVer[50] = "UNKNOWN";
   
   void set_mode(int mode); 
   void set_range(double r); 
   void set_range_min(); 
   void set_range_max(); 
   void set_level(double lvl); 
   void set_output_state(int state); 
   void set_clock_time(int hr,int min,int sec);  
   void set_clock_date(int month,int day,int year); 
   void write(const char *); 
  
   int open_vxi11_connection(const char *ip_addr); 
   int close_vxi11_connection(); 
   int self_test(); 
   int error_check(); 

   int get_output_state(); 
   int get_mode(); 
 
   double get_level(); 
   double get_range();
 
   char *ask(const char *); 
   char *get_device_id(); 
   char *get_clock_time(); 
   char *get_clock_date(); 

}

#endif  

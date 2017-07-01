// test the keithley comms 

#include <iostream>
#include <fstream>
#include <string>

#include "KeithleyInterface.hh"

int main(){

   std::string dev_name = "KEITHLEY";
   std::string dev_path = "/dev/usbtmc";

   int protocol = CommDriver::kUSBTMC;

   int portNo = keithley_interface::open_connection( protocol,dev_name.c_str(),dev_path.c_str() );

   int rc = keithley_interface::set_to_remote_mode(protocol,portNo);

   char msg[512];
   rc = keithley_interface::get_device_id(protocol,portNo,msg);
   std::cout << msg << std::endl;

   double maxRange = 100E+3;
   rc = keithley_interface::set_range(protocol,portNo,maxRange);

   double R=0;
   rc = keithley_interface::get_resistance(protocol,portNo,R);
   std::cout << "Resistance = " << R << std::endl;

   strcpy(msg,"");
   rc = keithley_interface::check_errors(protocol,portNo,msg);
   std::cout << "Error code " << rc << ": " << msg << std::endl;
   strcpy(msg,"");

   rc = keithley_interface::clear_errors(protocol,portNo);
   rc = keithley_interface::check_errors(protocol,portNo,msg);
   std::cout << "Error code " << rc << ": " << msg << std::endl;

   rc = keithley_interface::close_connection(protocol,portNo);

   return rc;
}


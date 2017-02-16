// a test program to talk to the Yokogawa 

#include "YokogawaInterface.hh" 

int main(int argc,char **argv){

   int rc=0; 

   const int SIZE = 13; 
   char *ip_addr = new char[SIZE+1]; 
   sprintf(ip_addr,"%s","192.168.5.160"); 

   std::cout << "Connecting to Yokogawa..." << std::endl;

   rc = yokogawa_interface::open_connection(ip_addr); 
   
   if (rc==0) { 
      std::cout << "Device connected! " << std::endl;
   } else { 
      std::cout << "Device connection FAILED.  Exiting... " << std::endl;
      return 1;
   }

   std::cout << "Checking the mode..." << std::endl;
   rc = yokogawa_interface::get_mode(); 
 
   if (rc==yokogawa_interface::kCURRENT) { 
      std::cout << "Device is in CURRENT mode" << std::endl;
   } else { 
      std::cout << "Device is in VOLTAGE mode" << std::endl;
   }

   std::cout << "Checking the output state..." << std::endl;
   rc = yokogawa_interface::get_output_state(); 

   if (rc==yokogawa_interface::kENABLED) { 
      std::cout << "Device output is ENABLED" << std::endl;
   } else { 
      std::cout << "Device output is DISABLED" << std::endl;
   }

   rc = yokogawa_interface::close_connection(); 

   delete ip_addr; 

   return 0;
}

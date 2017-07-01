#include "CommDriver.hh"
//______________________________________________________________________________
namespace CommDriver {
   //______________________________________________________________________________
   int open_connection(int type,const char *device_name,const char *device_path){
      int handle = -1;
      switch (type) {
         case CommDriver::kRS232:
            handle = rs232_open_connection(device_path);
            break;
         case CommDriver::kUSBTMC:
            handle = usbtmc_open_connection(device_name,device_path);
            break;
         case CommDriver::kTCPIP:
            break;
         default:
	    print_message("open_connection","Invalid protocol!"); 
      }
      return handle;
   }
   //______________________________________________________________________________
   int close_connection(int type,int handle){
      int rc=-1;
      switch (type) {
         case CommDriver::kRS232:
            rc = rs232_close_connection(handle);
            break;
         case CommDriver::kUSBTMC:
            rc = usbtmc_close_connection(handle);
            break;
         case CommDriver::kTCPIP:
            break;
         default:
	    print_message("close_connection","Invalid protocol!"); 
            rc = 1;
      }
      return rc;
   }
   //______________________________________________________________________________
   int write_cmd(int type,int handle,const char *buffer){
      int rc=-1;
      switch (type) {
         case CommDriver::kRS232:
            rc = rs232_write(handle,buffer);
            break;
         case CommDriver::kUSBTMC:
            rc = usbtmc_write(handle,buffer);
            break;
         case CommDriver::kTCPIP:
            break;
         default:
	    print_message("write_cmd","Invalid protocol!"); 
            rc = 1;
      }
      return rc;
   }
   //______________________________________________________________________________
   int query(int type,int handle,const char *cmd,char *response){
      int rc=-1;
       switch (type) {
         case CommDriver::kRS232:
            rc = rs232_query(handle,cmd,response);
            break;
         case CommDriver::kUSBTMC:
            rc = usbtmc_query(handle,cmd,response);
            break;
         case CommDriver::kTCPIP:
            break;
         default:
	    print_message("query","Invalid protocol!"); 
      }
      return rc;
   }
   //______________________________________________________________________________
   int rs232_open_connection(const char *device_path) {
      int rs232_handle=0;
      rs232_handle = open(device_path, O_RDWR | O_NOCTTY | O_NDELAY);
      // rs232_handle = open(device_path, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
      if (rs232_handle < 0) {
         print_message("rs232_open_connection","Error: Failed to open usb->serial port"); 
         return -1;
      }

      struct termios rs232_old_termios;
      if ( tcgetattr(rs232_handle, &rs232_old_termios) != 0 ) {
         print_message("rs232_open_connection","Error: Failed to read original serial settings"); 
         rs232_close_connection(rs232_handle);
         return -1;
      }

      // 8 data bits, no parity, 1 stop bit, 9600 baud, hdw flow control
      struct termios new_termios;
      // new_termios.c_cflag = CS8 | B9600 | CRTSCTS;
      // new_termios.c_cflag = CS8 | B115200 | CRTSCTS;
      new_termios.c_cflag     &=  ~PARENB;        // Make 8n1  
      new_termios.c_cflag     &=  ~CSTOPB;
      new_termios.c_cflag     &=  ~CSIZE;
      new_termios.c_cflag     |=  CS8;
      new_termios.c_cflag     &=  ~CRTSCTS;       // no flow control 
      // set baud rate 
      cfsetospeed(&new_termios,B115200);
      cfsetispeed(&new_termios,B115200);

      int rc = tcsetattr(rs232_handle, TCSANOW, &new_termios);
      char err_msg[512];
      if(rc<0){
         sprintf(err_msg,"Something's wrong. Error code: %d \n",rc);
         print_message("rs232_open_connection",err_msg);
         return -1;
      }

      tcflush(rs232_handle,TCIOFLUSH);

      usleep(1E5);
      return rs232_handle;
   }

   //______________________________________________________________________________
   int rs232_close_connection(int rs232_handle){
      // tcsetattr(rs232_handle,TCSANOW,&rs232_old_termios);      // restore old settings
      return close(rs232_handle);
   }
   //______________________________________________________________________________
   int rs232_write(int handle,const char *cmd){
      int rc = write( handle,cmd,strlen(cmd) );
      return rc;
   }
   //______________________________________________________________________________
   int rs232_query(int handle,const char *query,char *response){
      const int SIZE = 512;
      int qSIZE = (int)( strlen(query) );
      int rc = write(handle,query,qSIZE);
      if (rc==0) {
         rc = read(handle,response,SIZE);
      } else {
         print_message("rs232_query","Cannot write to device!"); 
      }
      return rc;
   }
   //______________________________________________________________________________
   int usbtmc_open_connection(const char *dev_name,const char *dev_path){
      int SIZE = 128;
      char DEV_PATH[SIZE],DEV_FULL_PATH[SIZE];
      sprintf(DEV_PATH,"%s0",dev_path);

      // FIXME: is there a better way to do this?  
      std::stringstream ss;
      std::string s;
      std::ifstream in(DEV_PATH);

      // look for the keithley 
      if( in.good() ){
         ss << in.rdbuf();
         in.close();
      }else{
         print_message("usb_open_connection","No USBTMC devices found"); 
         return 1;
      }

      for(int i=0;i<16;i++){
         std::getline(ss, s, '\n');
         if(s.find(dev_name) != s.size() - 1){
            std::cout << s << std::endl;
            sprintf(DEV_FULL_PATH, "/dev/usbtmc%i", i);
         }
      }

      int portNo=-1;
      portNo = open(DEV_FULL_PATH,O_RDWR);
      return portNo;
   }
   //______________________________________________________________________________
   int usbtmc_close_connection(int handle){
      int rc = close(handle);
      return rc;
   }
   //______________________________________________________________________________
   int usbtmc_write(int handle,const char *cmd){
      int rc = write( handle,cmd,strlen(cmd) );
      return rc;
   }
   //______________________________________________________________________________
   int usbtmc_ask(int portNo,const char *query,char *response){
      int rc=0;
      const int SIZE = 512;
      char write_msg[SIZE],read_msg[SIZE]; 
      int nBytes_wr = usbtmc_write(portNo,query);
      int nBytes_rd = read(portNo,response,SIZE);
      if(nBytes_wr==0||nBytes_rd==0){
         sprintf(write_msg,"Write response = %d",nBytes_wr);
         sprintf(read_msg ,"Read response  = %d",nBytes_rd);
         print_message("usbtmc_ask",write_msg);
         print_message("usbtmc_ask",read_msg);
         strcpy(response,"NO RESPONSE");    // comms failed  
         rc = 1;
      }
      return rc;
   }
   //______________________________________________________________________________
   int print_message(const char* func,const char *msg){
      char notification[512]; 
      sprintf(notification,"[CommDriver::%s]: %s",func,msg); 
      std::cout << notification << std::endl;
      return 0; 
   }
}

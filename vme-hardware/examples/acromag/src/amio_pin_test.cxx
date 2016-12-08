/*===========================================================================*\

author: Matthias W. Smith
email:  mwsmith2@uw.edu
file:   vme_amio_test.cxx

about:  This module is used to test the operation of the Acromag digital I/O
        boards.  It flips Pin 50 between +5V, 0V at a rate of 2 Hz.  One
	simply needs to measure the PIN outputs to confirm that it works.

\*===========================================================================*/

//--- std includes ----------------------------------------------------------//
#include <fcntl.h>
#include <cassert>
#include <unistd.h>

//--- project includes ------------------------------------------------------//
#include "g2field/acromag_ip470a.hh"
#include "g2field/common.hh"
using namespace hw;

int main(int argc, char* argv[])
{
  int vme_dev, rc;
  u_int8_t data;
  unsigned int status;
  long int address;

  int trg_port = 5;
  double wait_time = 2.5;
  
  // Create a Acromag IO object in carrier board position A.
  AcromagIp470a myam(0x0, BOARD_A);

  myam.CheckBoardId();
  myam.WritePort(7, 0x0);

  // Set a slow loop of on off switches to test things.
  for (int i = 0; i < 10000; ++i) {
    std::cout << "round: " << i << " - turning on pin " << i % 48 << std::endl;

    data = 1;
    int port = (i % 48) / 8;
    // Turn on the gates
   
    myam.WriteBit(port, i, data);
    usleep(wait_time * 1.0e6);

    data = 0;
    myam.WriteBit(port, i, data);
  }
  
  return 0;
}

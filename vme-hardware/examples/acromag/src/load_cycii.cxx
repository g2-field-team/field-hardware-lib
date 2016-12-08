/*===========================================================================*\

author: Matthias W. Smith
email:  mwsmith2@uw.edu

file:   cycii_load.cxx
about:  This class interacts with the Altera Cyclone II FPGA via IP-BUS (VME).

\*===========================================================================*/

//--- std includes ----------------------------------------------------------//
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <string>

//--- other includes --------------------------------------------------------//

//--- project includes ------------------------------------------------------//
#include "g2field/altera_cycii.hh"
#include "g2field/common.hh"

int main(int argc, char *argv[])
{
  // Assure that we have a hex file to load.
  if (argc < 1) {
    std::cout << "usage: ./cycii_load <hex_code_filename>" << std::endl;
    return 1;
  }

  // Create a Cyclone 2 object in carrier position D.
  hw::AlteraCycII fpga(0x0, hw::BOARD_D);
  fpga.CheckBoardId();

  // Try loading the parsing the hex code and configuring the FPGA.
  int rc = fpga.LoadHexCode(std::string(argv[1]));
  if (rc != 0) {
    std::cerr << "Could not load code onto FPGA." << std::endl;
  }

  return 0;
}

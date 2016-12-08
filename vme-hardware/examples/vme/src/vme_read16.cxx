/*==========================================================================*\

  author: Matthias W. Smith
  email:  mwsmith2@uw.edu
  file:   vme_read16.cxx

  about: A simple program that reads 2 bytes from a 2 byte VME address. 
         The program is for debugging purposes more than anything.

\*==========================================================================*/

//--- std includes ---------------------------------------------------------//
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <cassert>

//--- other includes -------------------------------------------------------//
#include "g2field/vme/sis3100_vme_calls.h"
#include "g2field/common.hh"

int main(int argc, char** argv)
{
  int vme_dev, rc;
  ushort state;
  long int address;

  // Make sure we have enough arguments.
  if (argc < 2) {
    printf("usage: ./vme_read16 <vme_address(hex)>");
  }
  assert(argc > 1);
  
  // Open a pipe to the vme crate in the usual location.
  vme_dev = open(hw::vme_path.c_str(), O_RDWR, 0);
  assert(vme_dev >= 0);
  
  // Parse the hex address and read from it.
  address = strtol(argv[1], NULL, 16);
  rc = vme_A16D16_read(vme_dev, address, &state);
  
  // Print the results.
  printf("device: %d, return: %d\n", vme_dev, rc);
  printf("address: 0x%08x\n", address);
  printf("state: 0x%02x-%02x-%02x-%02x\n", 
	 (state >> 24) ,(state >> 16) & 0xFF, 
	 (state >> 8) & 0xFF, state & 0xFF);

  return 0;
}

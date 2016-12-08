/*==========================================================================*\

  author: Matthias W. Smith
  email:  mwsmith2@uw.edu
  file:   vme_write.cxx

  about: A simple program that writes/reads 4 bytes from a 4 byte VME 
         address. The program is for debugging purposes more than anything.

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
  int vme_dev, ret;
  unsigned int status, data;
  long int address;

  // Make sure we have enough arguments.
  if (argc < 3) {
    printf("usage: ./vme_read <vme_address(hex)> <vme_message(hex)>");
  }
  assert(argc > 2);

  // Open a pipe to the vme crate in the usual location.
  vme_dev = open(hw::vme_path.c_str(), O_RDWR, 0);
  assert(vme_dev >= 0);
  
  // Parse the hex address and message to write.
  address = strtol(argv[1], NULL, 16);
  data = strtol(argv[2], NULL, 16);
  printf("Address: %ld\n", address);

  // Write the address then read the resulting state.
  ret = vme_A32D32_write(vme_dev, address, data);
  ret = vme_A32D32_read(vme_dev, address, &status);
  
  // Print the results.
  printf("device: %d\n", vme_dev);
  printf("return: %d\n", ret);
  printf("status: %02x %02x %02x %02x\n", 
	 (status >> 24) ,(status >> 16) & 0xFF, 
	 (status >> 8) & 0xFF, status & 0xFF);

  return 0;
}

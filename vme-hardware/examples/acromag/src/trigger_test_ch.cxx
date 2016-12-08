/*===========================================================================*\

author: Matthias W. Smith
email:  mwsmith2@uw.edu
file:   vme_trigger_test_ch.cxx

about:  Tests the vme nmr trigger functionality.

\*===========================================================================*/

//--- std includes ----------------------------------------------------------//
#include <fcntl.h>
#include <cassert>
#include <unistd.h>

//--- project includes ------------------------------------------------------//
#include "g2field/dio_trigger_board.hh"
#include "g2field/common.hh"

using namespace hw;

int main(int argc, char* argv[])
{
  int vme_dev, rc;
  u_int8_t data;
  unsigned int status;
  long int address;

  double pulse_width = 1000.0; // us
  double rate = 2.0; // Hz
  int trigger_mask = 0xff;
  int trigger_port = 3;

  if (argc > 1) {
    trigger_mask = 0x1 << atoi(argv[1]);
  }

  if (argc > 2) {
    trigger_port = atoi(argv[1]);
    trigger_mask = 0x1 << atoi(argv[2]);
  }

  // Create a Acromag IO object in carrier board position A.
  std::string conf_file("config/fe_vme_shimming.json");
  DioTriggerBoard trigger(0x0, BOARD_B, trigger_port, false);

  pulse_width = 0.5e6 / rate;

  for (int i = 0; i < rate * 100000; ++i) {
    trigger.FireTriggers(trigger_mask, pulse_width);
    usleep(1.0e6 / rate);
  }

  return 0;
}

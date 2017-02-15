/*===========================================================================*\

author: Matthias W. Smith
email:  mwsmith2@uw.edu
file:   vme_single_trigger.cxx

about:  Issues a single trigger.

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

  // Create a Acromag IO object in carrier board position A.
  std::string conf_file("config/fe_vme_shimming.json");
  DioTriggerBoard trigger(0x0, BOARD_B, 3, false);

  double pulse_width = 1000.0; // us
  int trigger_mask = 63;

  trigger.FireTriggers(trigger_mask, pulse_width);

  return 0;
}

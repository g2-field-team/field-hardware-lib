/*===========================================================================*\

author: Matthias W. Smith
email:  mwsmith2@uw.edu
file:   vme_amio_test.cxx

about:  Sets the channel on a multiplexer set on port 0 & 1 of board A.

\*===========================================================================*/

//--- std includes ----------------------------------------------------------//
#include <fcntl.h>
#include <cassert>
#include <unistd.h>

//--- project includes ------------------------------------------------------//
#include "g2field/dio_mux_controller.hh"
#include "g2field/common.hh"

using namespace hw;

int main(int argc, char* argv[])
{
  // Verify arguments.
  if (argc < 2) {
    std::cout << "Usage: ./set_uw_mux_ch <channel>" << std::endl;
    exit(1);
  }

  // Create a Acromag IO object in carrier board position A.
  DioMuxController mux_ctrl(0x0, BOARD_B, false);

  mux_ctrl.AddMux("mux_3", 2, false);
  mux_ctrl.SetMux("mux_3", std::stoi(argv[1]));

  return 0;
}

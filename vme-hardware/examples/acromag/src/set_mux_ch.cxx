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
  // Make sure we got enough arguments.
  if (argc < 3) {
    std::cout << "Usage: ./set_mux_ch <port> <channel>" << std::endl;
    exit(1);
  }

  // Create a Acromag IO object in carrier board position B.
  DioMuxController mux_ctrl(0x0, BOARD_B, false);
  mux_ctrl.AddMux("mux_1", std::stoi(argv[1]));
  mux_ctrl.SetMux("mux_1", std::stoi(argv[2]));

  return 0;
}

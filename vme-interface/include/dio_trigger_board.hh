#ifndef VME_HARDWARE_INCLUDE_DIO_TRIGGER_BOARD_HH_
#define VME_HARDWARE_INCLUDE_DIO_TRIGGER_BOARD_HH_

/*============================================================================*\

author: Matthias W. Smith
email:  mwsmith2@uw.edu
file:   dio_trigger_board.hh

about: Allows easy intuitive logic trigger based on acromag ip470
       digital I/O boards.

\*============================================================================*/

//--- std includes -----------------------------------------------------------//
#include <unistd.h>
#include <string>

//--- other includes ---------------------------------------------------------//

//--- project includes -------------------------------------------------------//
#include "acromag_ip470a.hh"

namespace hw {

class DioTriggerBoard
{
 public:

  // Ctor params:
  DioTriggerBoard(int board_addr,
                  board_id bid,
                  int trg_port,
                  bool use_sextets=true);

  // Set the proper acromag port used for sending TTL triggers.
  void SetTriggerPort(int trg_port) { trg_port_ = trg_port; };

  // Set the 
  void SetTriggerMask(int trg_mask) { trg_mask_ = trg_mask; };

  // Fire TTL pulse
  int FireTrigger(int trg_bit=0, int length_us=0);

  // Fire TTL pulses
  int FireTriggers(int trg_mask=0, int length_us=0);

 private:

  AcromagIp470a io_board_;
  int trg_port_;
  int trg_mask_;
};

} // ::hw

#endif

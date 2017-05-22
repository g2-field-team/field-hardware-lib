#include "dio_trigger_board.hh"

namespace hw {

DioTriggerBoard::DioTriggerBoard(int board_addr,
                                 board_id bid,
                                 int trg_port,
                                 bool use_sextets) :
  io_board_(board_addr, bid, use_sextets), 
  trg_port_(trg_port), trg_mask_(0xff) {}

int DioTriggerBoard::FireTrigger(int trg_bit, int length_us)
{
  // Get the current state and only flip the trigger bit.
  int rc = 0;
  u_int8_t data;
  rc += io_board_.ReadPort(trg_port_, data);
  data &= ~(0x1 << trg_bit);

  // Start the trigger and wait the allotted pulse time.
  rc += io_board_.WritePort(trg_port_, data);
  usleep(length_us);

  // Now turn the trigger bit back off.
  rc += io_board_.ReadPort(trg_port_, data);
  data |= 0x1 << trg_bit;
  rc += io_board_.WritePort(trg_port_, data);

  return rc;
}

int DioTriggerBoard::FireTriggers(int trg_mask, int length_us)
{
  // Get the current state and only flip the trigger bit.
  int rc = 0;
  u_int8_t data;

  if (trg_mask == 0) {
    trg_mask = trg_mask_;
  }

  // Start the trigger and wait the allotted pulse time.
  rc += io_board_.ReadPort(trg_port_, data);
  data &= ~trg_mask;

  rc += io_board_.WritePort(trg_port_, data);
  
  if (length_us > 0) {
    usleep(length_us); 
  }

  // Now turn the trigger bit back off.
  rc += io_board_.ReadPort(trg_port_, data);
  data |= trg_mask;
  rc += io_board_.WritePort(trg_port_, data);

  return rc;
}

} // ::hw

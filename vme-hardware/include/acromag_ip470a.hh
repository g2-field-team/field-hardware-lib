#ifndef VME_HARDWARE_INCLUDE_ACROMAG_IP470A_HH_
#define VME_HARDWARE_INCLUDE_ACROMAG_IP470A_HH_

/*===========================================================================*\

author: Matthias W. Smith
email:  mwsmith2@uw.edu
file:   acromag_ip470a.hh

about:  The code in this file wraps the basic functionality of Acromag IP470A
        digitial io boards.  The boards live on a vme carrier board.

\*===========================================================================*/

//--- std includes ----------------------------------------------------------//
#include <iostream>

//--- other includes --------------------------------------------------------//
#include "common.hh"
#include "common_base.hh"
#include "sis3100_vme_dev.hh"

//--- project includes ------------------------------------------------------//

namespace hw {

enum board_id {BOARD_A, BOARD_B, BOARD_C, BOARD_D};

class AcromagIp470a : public Sis3100VmeDev {

 public:

  // ctor params:
  //   dev - handle to vme device
  //   base_addr - base address of vme device
  //   block - used to calcuate base address, acromag blocks A-D 
  //           get 0xff address space
  //   16 - specifies A16 vme addressing
  //   32 - specifies BLT32 vme block transfers
  AcromagIp470a(int carrier_address, board_id block, bool use_sextets=false) : 
    use_sextets_(use_sextets), 
    Sis3100VmeDev(carrier_address + 0x100 * block, 16, 32) {
    name_ = std::string("AcromagIp470a_") + std::to_string(block);
  };

  // Formats and prints the ID data in the ID register of the board.
  void CheckBoardId();

  // A member method to enable blocks of 6.
  void EnableSextets() { use_sextets_ = true; };

  // A member method to enable blocks of 8.
  void EnableOctets() { use_sextets_ = false; };

  // Enhanced Mode opens more registers via a shift bit.  It can be
  // used to change the amplitude and interrupt nature of the IO board.
  void ActivateEnhancedMode();

  // Check the status of a one byte port, 0-7, 0-6 are again IO pins.
  int ReadPort(int port_id);
  int ReadPort(int port_id, u_int8_t &data);

  // Write data to a one byte port, 0-7, 0-6 of which control 48 IO pins.
  int WritePort(int port_id);
  int WritePort(int port_id, u_int8_t data);

  // Read a single bit on a specific port.
  int ReadBit(int port_id, int bit_id, u_int8_t &data);

  // Write a single bit of data to a specified port, preserving other data.
  int WriteBit(int port_id, int bit_id, u_int8_t data);

 private:
  
  bool is_enhanced_;
  bool use_sextets_;
  u_int8_t data_; // force D8 vme reads/writes.
  std::string name_;

  int ReadOctet(int block_idx);
  int ReadOctet(int block_idx, u_int8_t &data);
  
  int WriteOctet(int block_idx);
  int WriteOctet(int block_idx, u_int8_t data);

  int ReadSextet(int block_idx);
  int ReadSextet(int block_idx, u_int8_t &data);
  
  int WriteSextet(int block_idx);
  int WriteSextet(int block_idx, u_int8_t data);
};

} // ::hw

#endif
  

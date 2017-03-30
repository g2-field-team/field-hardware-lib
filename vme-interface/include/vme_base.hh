#ifndef VME_HARDWARE_INCLUDE_VME_BASE_HH_
#define VME_HARDWARE_INCLUDE_VME_BASE_HH_

/*===========================================================================*\

  author: Matthias W. Smith
  email:  mwsmith2@uw.edu
  file:   vme_device.hh

  about:  Implements the some basic vme functionality to form a base
          class that vme devices can inherit.  It really just defines
	  the most basic read, write, and block transfer vme functions.
	  Sis3100VmeDev is a better class to inherit from if more
	  functionality is needed.

\*===========================================================================*/

//--- std includes ----------------------------------------------------------//
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <errno.h>

//--- other includes --------------------------------------------------------//
#include "vme/sis3100_vme_calls.h"
#include "TFile.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

//--- project includes ------------------------------------------------------//
#include "common_base.hh"
#include "common.hh"

namespace hw {

// This class pulls data from a vme device.
// The template is the data structure for the device.
class VmeBase : virtual public CommonBase {

public:

  // Ctor params:
  // name - used in naming output data
  // conf - load parameters from a json configuration file
  // num_ch_ - number of channels in the digitizer
  // read_trace_len - length of each trace in units of sizeof(uint)
  VmeBase(std::string name, std::string conf) : CommonBase(name) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(conf, pt);
    // Get the base address for the device.  Convert from hex.
    base_address_ = std::stoul(pt.get<std::string>("base_address"), nullptr, 0);
  };

  // VmeBase(std::string name, boost::property_tree::ptree pt) :
  // CommonBase(name, conf);

protected:

  const int max_read_attempts_ = 100;
  const int wait_time_us_ = 50;

  int device_;
  int read_len_;
  uint base_address_; // contained in the conf file.

  inline int Read(uint addr, uint &msg);        // A32D32
  inline int Write(uint addr, uint msg);        // A32D32
  inline int Read16(uint addr, ushort &msg);    // A16D16
  inline int Write16(uint addr, ushort msg);    // A16D16
  inline int ReadTrace(uint addr, uint *trace); // 2eVME (A32)
  inline int ReadTrace(uint addr, uint *trace, int trace_len); // 2eVME (A32)
  inline int ReadTraceFifo(uint addr, uint *trace); // 2eVMEFIFO (A32)
  inline int ReadTraceFifo(uint addr, uint *trace, int trace_len); // 2eVMEFIFO (A32)
  inline int ReadTraceDma32Fifo(uint addr, uint *trace);
  inline int ReadTraceDma32Fifo(uint addr, uint *trace, int trace_len);
  inline int ReadTraceMblt64(uint addr, uint *trace, int trace_len);
  inline int ReadTraceMblt64(uint addr, uint *trace);
  inline int ReadTraceMblt64SameBlock(uint addr, uint *trace, int trace_len);
  inline int ReadTraceMblt64SameBlock(uint addr, uint *trace);
  inline int ReadTraceMblt64Fifo(uint addr, uint *trace, int trace_len);
  inline int ReadTraceMblt64Fifo(uint addr, uint *trace);
};

// Reads 4 bytes from the specified address offset.
//
// params:
//   addr - address offset from base_addr_
//   msg - catches the read data
//
// return:
//   error code from vme read
int VmeBase::Read(uint addr, uint &msg)
{
  static int retval, status, count;

  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }

  status = (retval = vme_A32D32_read(device_, base_address_ + addr, &msg));
  close(device_);

  if (status != 0) {
    this->LogError("read32  failure at address 0x%08x", base_address_ + addr);

  } else {

    this->LogDump("read32  vme device 0x%08x, register 0x%08x, data 0x%08x",
                   base_address_, addr, msg);
  }

  return retval;
}


// Writes 4 bytes from the specified address offset.
//
// params:
//   addr - address offset from base_addr_
//   msg - data to be written
//
// return:
//   error code from vme write

int VmeBase::Write(uint addr, uint msg)
{
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }

  // Make the vme call.
  status = (retval = vme_A32D32_write(device_, base_address_ + addr, msg));
  close(device_);

  if (status != 0) {
    this->LogError("write32 failure at address 0x%08x", base_address_ + addr);

  } else {

    this->LogDump("write32 vme device 0x%08x, register 0x%08x, data 0x%08x",
                   base_address_, addr, msg);
  }

  return retval;
}


// Reads 2 bytes from the specified address offset.
//
// params:
//   addr - address offset from base_addr_
//   msg - catches the read data
//
// return:
//   error code from vme read
int VmeBase::Read16(uint addr, ushort &msg)
{
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }

  status = (retval = vme_A32D16_read(device_, base_address_ + addr, &msg));
  close(device_);

  if (status != 0) {
    this->LogError("read16  failure at address 0x%08x", base_address_ + addr);

  } else {

    this->LogDump("read16  vme device 0x%08x, register 0x%08x, data 0x%04x",
                   base_address_, addr, msg);
  }

  return retval;
}


// Writes 2 bytes from the specified address offset.
//
// params:
//   addr - address offset from base_addr_
//   msg - data to be written
//
// return:
//   error code from vme read
int VmeBase::Write16(uint addr, ushort msg)
{
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }

  // Make our vme call.
  status = (retval = vme_A32D16_write(device_, base_address_ + addr, msg));
  close(device_);

  if (status != 0) {
    this->LogError("write16 failure at address 0x%08x", base_address_ + addr);

  } else {

    this->LogDump("write16 vme device 0x%08x, register 0x%08x, data 0x%04x",
                   base_address_, addr, msg);
  }

  return retval;
}


// Reads a block of data from the specified address offset.  The total
// number of bytes read depends on the trace_len variable.
//
// params:
//   addr - address offset from base_addr_
//   trace - pointer to data being read
//
// return:
//   error code from vme read
int VmeBase::ReadTrace(uint addr, uint *trace)
{
  static uint num_got;
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }

  // Make the vme call.
  this->LogDump("read_2evme vme device 0x%08x, register 0x%08x, samples %i",
		 base_address_, addr, read_len_);

  status = (retval = vme_A32_2EVME_read(device_,
                                        base_address_ + addr,
                                        trace,
                                        read_len_,
                                        &num_got));
  close(device_);

  if (status != 0) {
    this->LogError("read32_evme failed at 0x%08x", base_address_ + addr);

  } else {

    this->LogDump("read32_evme address 0x%08x, ndata asked %i, ndata recv %i",
                   base_address_ + addr, read_len_, num_got);
  }

  return retval;
}

int VmeBase::ReadTrace(uint addr, uint *trace, int trace_len)
{
  static uint num_got;
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }

  // Make the vme call.
  this->LogDump("read_2evme vme device 0x%08x, register 0x%08x, samples %i",
		 base_address_, addr, trace_len);

  status = (retval = vme_A32_2EVME_read(device_,
                                        base_address_ + addr,
                                        trace,
                                        trace_len,
                                        &num_got));
  close(device_);

  if (status != 0) {
    this->LogError("read32_evme failed at 0x%08x", base_address_ + addr);

  } else {

    this->LogDump("read32_evme address 0x%08x, ndata asked %i, ndata recv %i",
                   base_address_ + addr, trace_len, num_got);
  }

  return retval;
}


int VmeBase::ReadTraceFifo(uint addr, uint *trace)
{
  static uint num_got;
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }

  // Make the vme call.
  status = (retval = vme_A32_2EVMEFIFO_read(device_,
         				    base_address_ + addr,
         				    trace,
         				    read_len_,
         				    &num_got));
  close(device_);

  if (status != 0) {
    this->LogError("read32_2evmefifo failed at 0x%08x", base_address_ + addr);

  } else {

    this->LogDump("read32_2evmefifo addr 0x%08x, trace_len %i, ndata recv %i",
                   base_address_ + addr, read_len_, num_got);
  }

  return retval;
}

int VmeBase::ReadTraceFifo(uint addr, uint *trace, int trace_len)
{
  static uint num_got;
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }

  // Make the vme call.
  status = (retval = vme_A32_2EVMEFIFO_read(device_,
         				    base_address_ + addr,
         				    trace,
         				    trace_len,
         				    &num_got));
  close(device_);

  if (status != 0) {
    this->LogError("read32_2evmefifo failed at 0x%08x", base_address_ + addr);

  } else {

    this->LogDump("read32_2evmefifo addr 0x%08x, trace_len %i, ndata recv %i",
                   base_address_ + addr, trace_len, num_got);
  }

  return retval;
}


// Reads a block of data from the specified address offset.  The total
// number of bytes read depends on the trace_len variable. Uses MBLT64
//
// params:
//   addr - address offset from base_addr_
//   trace - pointer to data being read
//
// return:
//   error code from vme read
int VmeBase::ReadTraceMblt64(uint addr, uint *trace)
{
  static uint num_got;
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }

  // Make the vme call.
  status = (retval = vme_A32MBLT64_read(device_,
                                        base_address_ + addr,
                                        trace,
                                        read_len_,
                                        &num_got));
  close(device_);

  if (status != 0) {
    this->LogError("readA32_mblt64 failed at 0x%08x, asked: %i, recv: %i, retval: %i",
                    base_address_ + addr, read_len_, num_got, retval);

  } else {

    this->LogDump("read32_mblt address 0x%08x, ndata asked %i, ndata recv %i",
                   base_address_ + addr, read_len_, num_got);
  }

  return retval;
}

int VmeBase::ReadTraceMblt64(uint addr, uint *trace, int trace_len)
{
  static uint num_got;
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }

  // Make the vme call.
  status = (retval = vme_A32MBLT64_read(device_,
                                        base_address_ + addr,
                                        trace,
                                        trace_len,
                                        &num_got));
  close(device_);

  if (status != 0) {
    this->LogError("readA32_mblt64 failed at 0x%08x, asked: %i, recv: %i, retval: %i",
                    base_address_ + addr, trace_len, num_got, retval);

  } else {

    this->LogDump("read32_mblt address 0x%08x, ndata asked %i, ndata recv %i",
                   base_address_ + addr, trace_len, num_got);
  }

  return retval;
}

// Reads a block of data from the specified address offset.  The total
// number of bytes read depends on the trace_len variable. Uses MBLT64
// The block transfers start from the same address
//
// params:
//   addr - address offset from base_addr_
//   trace - pointer to data being read
//
// return:
//   error code from vme read
int VmeBase::ReadTraceMblt64SameBlock(uint addr, uint *trace)
{
  static uint num_got;
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }


  int word_count = read_len_;
  unsigned int num_to_read;
  unsigned int offset = 0;
  //keep reading until it fails
  do {
    num_to_read = 0x0400;

    //retval = vme_A32MBLT64_read(device_,
    retval = vme_A32_2EVME_read(device_,
                                base_address_ + addr,
                                &trace[offset],
                                num_to_read,
                                &num_got);

    offset += num_got;
    word_count -= num_got;
  }
  while (word_count > 0 && retval == 0);

  //if (retval) {
  //	std::cout << "read_trace_len: " << trace_len << ", word count left: " << word_count << std::endl;
  //      std::cout << "retval: " << retval << ", num_got: " << num_got << ", at offset: " << offset << std::endl;
  //}

  //last transfer is BERR terminated
  status = -retval;
  if (offset > 0x0400) { status = offset; }


  close(device_);

  if (status < 0) {
    //this->LogError("readA32_mblt64 failed at 0x%08x, asked: %i, recv: %i, retval: %i, word count left: %i",
    //                base_address_ + addr, 0x0400, num_got, retval, word_count);

  } else {

    this->LogDump("read32_mblt address 0x%08x, ndata asked %i, ndata recv %i",
                   base_address_ + addr, read_len_, num_got);
  }

  return status;
}

int VmeBase::ReadTraceMblt64SameBlock(uint addr, uint *trace, int trace_len)
{
  static uint num_got;
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }


  int word_count = trace_len;
  unsigned int num_to_read;
  unsigned int offset = 0;
  //keep reading until it fails
  do {
    num_to_read = 0x0400;

    //retval = vme_A32MBLT64_read(device_,
    retval = vme_A32_2EVME_read(device_,
                                base_address_ + addr,
                                &trace[offset],
                                num_to_read,
                                &num_got);

    offset += num_got;
    word_count -= num_got;
  }
  while (word_count > 0 && retval == 0);

  //if (retval) {
  //	std::cout << "read_trace_len: " << trace_len << ", word count left: " << word_count << std::endl;
  //      std::cout << "retval: " << retval << ", num_got: " << num_got << ", at offset: " << offset << std::endl;
  //}

  //last transfer is BERR terminated
  status = -retval;
  if (offset > 0x0400) { status = offset; }


  close(device_);

  if (status < 0) {
    //this->LogError("readA32_mblt64 failed at 0x%08x, asked: %i, recv: %i, retval: %i, word count left: %i",
    //                base_address_ + addr, 0x0400, num_got, retval, word_count);

  } else {

    this->LogDump("read32_mblt address 0x%08x, ndata asked %i, ndata recv %i",
                   base_address_ + addr, trace_len, num_got);
  }

  return status;
}


// Reads a block of data from the specified address offset.  The total
// number of bytes read depends on the trace_len variable. Uses MBLT64
//
// params:
//   addr - address offset from base_addr_
//   trace - pointer to data being read
//
// return:
//   error code from vme read
int VmeBase::ReadTraceMblt64Fifo(uint addr, uint *trace)
{
  static uint num_got;
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }

  // Make the vme call.
  status = (retval = vme_A32MBLT64FIFO_read(device_,
         				    base_address_ + addr,
         				    trace,
         				    read_len_,
         				    &num_got));
  close(device_);

  if (status != 0) {
    this->LogError("read32_mblt_fifo failed at 0x%08x", base_address_ + addr);

  } else {

    this->LogDump("read32_mblt_fifo addr 0x%08x, trace_len %i, ndata recv %i",
                   base_address_ + addr, read_len_, num_got);
  }

  return retval;
}

int VmeBase::ReadTraceMblt64Fifo(uint addr, uint *trace, int trace_len)
{
  static uint num_got;
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }

  // Make the vme call.
  status = (retval = vme_A32MBLT64FIFO_read(device_,
         				    base_address_ + addr,
         				    trace,
         				    trace_len,
         				    &num_got));
  close(device_);

  if (status != 0) {
    this->LogError("read32_mblt_fifo failed at 0x%08x", base_address_ + addr);

  } else {

    this->LogDump("read32_mblt_fifo addr 0x%08x, trace_len %i, ndata recv %i",
                   base_address_ + addr, trace_len, num_got);
  }

  return retval;
}

// Reads a block of data from the specified address offset.  The total
// number of bytes read depends on the trace_len variable. Uses MBLT64
//
// params:
//   addr - address offset from base_addr_
//   trace - pointer to data being read
//
// return:
//   error code from vme read
int VmeBase::ReadTraceDma32Fifo(uint addr, uint *trace)
{
  static uint num_got;
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }

  // Make the vme call.
  status = (retval = vme_A32DMA_D32FIFO_read(device_,
					   base_address_ + addr,
					   trace,
					   read_len_,
					   &num_got));
  close(device_);

  if (status != 0) {
    this->LogError("read32_blt32_fifo failed at 0x%08x, trace_len: %i, num got: %i, retval: %i",
                    base_address_ + addr, read_len_, num_got, retval);

  } else {

    this->LogDump("read32_blt32_fifo addr 0x%08x, trace_len %i, ndata recv %i",
                   base_address_ + addr, read_len_, num_got);
  }

  return retval;
}

int VmeBase::ReadTraceDma32Fifo(uint addr, uint *trace, int trace_len)
{
  static uint num_got;
  static int retval, status, count;

  // Get the vme device handle.
  count = 0;
  do {
    device_ = open(hw::vme_path.c_str(), O_RDWR);
    usleep(wait_time_us_);
  } while ((device_ < 0) && (count++ < max_read_attempts_));

  // Log an error if we couldn't open it at all.
  if (device_ < 0) {
    this->LogError("failure to open device: %s", strerror(errno));
    return device_;
  }

  // Make the vme call.
  status = (retval = vme_A32DMA_D32FIFO_read(device_,
					   base_address_ + addr,
					   trace,
					   trace_len,
					   &num_got));
  close(device_);

  if (status != 0) {
    this->LogError("read32_blt32_fifo failed at 0x%08x, trace_len: %i, num got: %i, retval: %i",
                    base_address_ + addr, trace_len, num_got, retval);

  } else {

    this->LogDump("read32_blt32_fifo addr 0x%08x, trace_len %i, ndata recv %i",
                   base_address_ + addr, trace_len, num_got);
  }

  return retval;
}

} // ::hw

#endif

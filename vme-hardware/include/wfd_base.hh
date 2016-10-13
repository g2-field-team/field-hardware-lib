#ifndef VME_HARDWARE_INCLUDE_WFD_BASE_HH_
#define VME_HARDWARE_INCLUDE_WFD_BASE_HH_

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
#include <queue>
#include <thread>
#include <atomic>

//--- other includes --------------------------------------------------------//
#include "TFile.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

//--- project includes ------------------------------------------------------//
#include "common_base.hh"

namespace hw {

// This class pulls data from a vme device.
// The template is the data structure for the device.
class WfdBase : public virtual CommonBase {

public:
  
  // Ctor params:
  // name - used in naming output data
  // conf - load parameters from a json configuration file
  // num_ch_ - number of channels in the digitizer
  // read_trace_len_ - length of each trace in units of sizeof(uint)
  WfdBase(std::string name, 
	  std::string conf, 
	  int num_ch, 
	  int trace_len) : 
    CommonBase(name), num_ch_(num_ch), trace_len_(trace_len) {
    StartThread();
  };

  WfdBase(std::string name, 
	  boost::property_tree::ptree pt, 
	  int num_ch, 
	  int trace_len) : 
    CommonBase(name), num_ch_(num_ch), trace_len_(trace_len) {
    StartThread();
  };

  virtual ~WfdBase() {
    thread_live_ = false;
    if (work_thread_.joinable()) {
      work_thread_.join();
    }
  };

  // Spawns a new thread that pull in new data.
  virtual void StartThread() {
    thread_live_ = true;
    if (work_thread_.joinable()) {
      work_thread_.join();
    }
    std::cout << "Launching worker thread. " << std::endl;
    work_thread_ = std::thread(&WfdBase::WorkLoop, this);
  };

  // Rejoins the data pulling thread.
  virtual void StopThread() {
    thread_live_ = false;
    if (work_thread_.joinable()) {
      work_thread_.join();
    }
  };

  inline bool DataAvailable() {
    if (data_queue_.size() > 0) {
      return true;
    } else {
      return false;
    }
  }

protected:

  const int num_ch_;
  const uint trace_len_;

  std::atomic<bool> thread_live_; // keeps paused thread alive
  std::atomic<bool> go_time_;     // controls data taking
  std::atomic<bool> has_event_;   // useful for event building
  std::atomic<int> num_events_;   // useful for synchronization

  std::queue<wfd_data_t> data_queue_;      // stack to hold device events
  std::mutex queue_mutex_;        // mutex to protect data
  std::thread work_thread_;       // thread to launch work loop

  virtual bool EventAvailable() = 0;
  virtual void WorkLoop() = 0;
};

} // ::hw

#endif

#ifndef VME_HARDWARE_INCLUDE_COMMON_HH_
#define VME_HARDWARE_INCLUDE_COMMON_HH_

/*===========================================================================*\

author: Matthias W. Smith
email:  mwsmith2@uw.edu
file:   common.hh

about:  Contains the data structures for several hardware devices in a single
        location.  The header should be included in any program that aims
        to interface with (read or write) data with this hw.

\*===========================================================================*/

//--- std includes ----------------------------------------------------------//
#include <unistd.h>
#include <sys/time.h>
#include <cstdarg>
#include <chrono>
#include <vector>

//--- other includes --------------------------------------------------------//
#include "TFile.h"

namespace hw {

// Define a generic container for waveform digitizer data.
struct wfd_data_t {
  ULong64_t sys_clock;
  UInt_t trace_length;
  Double_t sampling_rate;
  std::vector<ULong64_t> dev_clock;
  std::vector<std::vector<UShort_t>> trace;
};

// Define a convenience type for dealing with groups of WFDs.
typedef std::vector<wfd_data_t> event_data_t;

// A useful define guard for I/O with the vme bus.
extern int vme_dev;
extern std::string vme_path;

// Create a variable for a config directory.
extern std::string conf_dir;

// Set sleep times for data polling threads.
const int short_sleep = 500;
const int long_sleep = 5000;
const double sample_period = 0.0001; // in milliseconds

inline void light_sleep() {
  usleep(short_sleep); // in usec
}

inline void heavy_sleep() {
  usleep(long_sleep); // in usec
}

inline long long systime_us() {
 static timeval t;
 gettimeofday(&t, nullptr);
 return (long long)(t.tv_sec)*1000000 + (long long)t.tv_usec;
}

inline long long steadyclock_us() {
 static timespec t;

 clock_gettime(CLOCK_MONOTONIC, &t);
 return (long long)(t.tv_sec)*1000000 + (long long)(t.tv_nsec * 0.001);
}

inline void wait_ns(int delta_t, int resolution=100) 
{
  using namespace std::chrono;
  auto t0 = steady_clock::now();
  auto t1 = steady_clock::now();
  struct timespec step;
  step.tv_sec = resolution / 1000000000;
  step.tv_nsec = resolution % 1000000000;
  
  while (duration_cast<nanoseconds>(t1 - t0).count() < delta_t) {
    nanosleep(&step, nullptr);
    t1 = steady_clock::now();
  }
}

} // ::hw

#endif

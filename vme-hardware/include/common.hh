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
#include <vector>
#include <array>
#include <mutex>
#include <cstdarg>
#include <unistd.h>
#include <sys/time.h>

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

// A useful define guard for I/O with the vme bus.
extern int vme_dev;
extern std::string vme_path;

// Create a variable for a config directory.
extern std::string conf_dir;

// Set sleep times for data polling threads.
const int short_sleep = 10;
const int long_sleep = 100;
const double sample_period = 0.0001; // in milliseconds

inline void light_sleep() {
 usleep(200); // in usec
}

inline void heavy_sleep() {
usleep(10000); // in usec
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

} // ::hw

#endif

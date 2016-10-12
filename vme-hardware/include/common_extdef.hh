#ifndef VME_HARDWARE_INCLUDE_COMMON_EXTDEF_HH_
#define VME_HARDWARE_INCLUDE_COMMON_EXTDEF_HH_

/*===========================================================================*\

author: Matthias W. Smith
email:  mwsmith2@uw.edu
file:   common_extdef.hh

about:  Exists to instantiate some of the shared variables for the hw.

\*===========================================================================*/

#include "common.hh"

namespace hw {

int vme_dev = -1;
std::string vme_path("/dev/sis1100_00remote");
std::mutex vme_mutex;

// Set the default config directory.
std::string conf_dir("/usr/local/etc/g2field/");

// Set the default logging behavior
int CommonBase::logging_verbosity_ = 3;
std::string CommonBase::logfile_("/usr/local/var/log/g2field/vme-hw.log");
std::fstream CommonBase::logstream_(logfile_);
std::mutex CommonBase::log_mutex_;

} // ::hw

#endif

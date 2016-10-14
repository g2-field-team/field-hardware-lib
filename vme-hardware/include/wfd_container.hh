#ifndef DAQ_FAST_CORE_INCLUDE_WORKER_LIST_HH_
#define DAQ_FAST_CORE_INCLUDE_WORKER_LIST_HH_

/*===========================================================================*\

  author: Matthias W. Smith
  email:  mwsmith2@uw.edu
  file:   wfd_container.hh
  
  about:  Creates a container class that can hold all WFD devices. This
          class makes event building and run flow much simpler to handle.
	  Additionally if new hardware is to be integrated it needs to be
	  added to this class.

\*===========================================================================*/

//--- std includes ----------------------------------------------------------//
#include <vector>

//--- project includes ------------------------------------------------------//
#include "common.hh"
#include "sis3302.hh"
#include "sis3316.hh"
#include "sis3350.hh"

namespace hw {

class WfdContainer : public CommonBase {

 public:

  // ctor
  WfdContainer() : CommonBase("WfdContainer") {};

  // dtor - the WfdContainer takes ownership of workers appended to
  // its worker vector.  They can be freed externally, but we need to
  // be sure.
  ~WfdContainer() {
    if (workers_.size() != 0) {
      FreeList();
    }
  };

  // Launches worker threads and starts the collection of data
  void StartRun();

  // Rejoins worker threads and stops the collection of data.
  void StopRun();

  // Launches worker threads.
  void StartThreads();

  // Stops and rejoins worker threads.
  void StopThreads();

  // Starts each worker collecting data.
  void StartWorkers();

  // Stops each worker from collecting data.
  void StopWorkers();

  // Checks if all workers have at least one event.
  bool AllWorkersHaveEvent();

  // Checks if any workers have a single event.
  bool AnyWorkersHaveEvent();

  // Checks if any workers have more than a single event.
  bool AnyWorkersHaveMultiEvent();

  // Copies event data into bundle.
  void GetEventData(event_data_t &bundle);

  // Flush all stale events.  Each worker has no events after this.
  void FlushEventData();

  // Add a newly allocated worker to the current list.
  void PushBack(WfdBase *worker) {
    workers_.push_back(worker);
  }

  // Deallocates each worker.
  void FreeList();
    
  // Return the size of the list.
  int Size() { return workers_.size(); };
  void Resize(int size) { workers_.resize(0); };

 private:

  // This is the actual worker list.
  std::vector<hw::WfdBase *> workers_;
};

} // ::hw

#endif

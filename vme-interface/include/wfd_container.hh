#ifndef VME_INTERFACE_INCLUDE_WFD_CONTAINER_HH_
#define VME_INTERFACE_INCLUDE_WFD_CONTAINER_HH_

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
#include "common_base.hh"
#include "wfd_base.hh"

namespace hw {

class WfdContainer : public CommonBase {

 public:

  // ctor
  WfdContainer() : CommonBase("WfdContainer") {};

  // dtor - the WfdContainer takes ownership of its workers
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

  // Trigger the WFDs via software.
  void SoftwareTriggers();

  // Copies event data into bundle.
  void GetEventData(event_data_t &bundle);

  // Flush all stale events.  Each worker has no events after this.
  void FlushEventData();

  // Add a newly allocated worker to the current list.
  void PushBack(WfdBase *worker) {
    workers_.push_back(worker);
  }

  // Deallocate each worker.
  void FreeList();
    
  // Return the size of the list.
  int Size() { return workers_.size(); };
  void Resize(int size) { workers_.resize(0); };

  // Create accessor for worker.
  inline const WfdBase *operator[] (int i) {
    return workers_[i];
  };

 private:

  // The actual worker list.
  std::vector<WfdBase *> workers_;
};

} // ::hw

#endif

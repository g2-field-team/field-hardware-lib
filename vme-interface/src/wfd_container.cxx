#include "wfd_container.hh"

namespace hw {

void WfdContainer::StartRun()
{
  StartThreads();
  StartWorkers();
}


void WfdContainer::StopRun()
{
  StopWorkers();
  StopThreads();
}


void WfdContainer::StartWorkers()
{
  LogMessage("Starting workers");
  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    (*it)->StartWorker();
  }
}


void WfdContainer::StartThreads()
{
  LogDebug("Launching worker threads");
  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    (*it)->StartThread();
  }
}


void WfdContainer::StopWorkers()
{
  LogMessage("Stopping workers");
  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    (*it)->StopWorker();
  }
}


void WfdContainer::StopThreads()
{
  LogDebug("Stopping worker threads");
  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    (*it)->StopThread();
  }
}


bool WfdContainer::AllWorkersHaveEvent()
{
  LogDebug("Checking if all workers have an event");
  bool has_event = true;

  // If any worker doesn't have an event, has_event will become false.
  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    has_event &= (*it)->has_event();
  }

  return has_event;
}


bool WfdContainer::AnyWorkersHaveEvent()
{
  LogDebug("Checking if any workers have an event");
  bool any_events = false;

  // A bitwise or here works, so that any event will return positive.
  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    any_events |= (*it)->has_event();
  }

  return any_events;
}


bool WfdContainer::AnyWorkersHaveMultiEvent()
{
  LogDebug("Checking if some workers have multiple events");
  int num_events = 0;

  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    num_events = (*it)->num_events();
    if (num_events > 1) return true;
  }

  return false;
}


void WfdContainer::SoftwareTriggers()
{
  LogDebug("Issuing software trigger to workers");
  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    (*it)->SoftwareTrigger();
  }
}


void WfdContainer::GetEventData(event_data_t &bundle)
{
  LogMessage("Collecting data from workers");
  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    bundle.push_back((*it)->PopEvent());
  }
}


void WfdContainer::FlushEventData()
{
  LogDebug("Flushing stale events from workers");
  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    (*it)->FlushEvents();
  }
}


void WfdContainer::FreeList()
{
  LogDebug("Freeing workers");
  for (auto it = workers_.begin(); it != workers_.end(); ++it) {       
    delete *it;
  }

  Resize(0);
}

} // ::hw

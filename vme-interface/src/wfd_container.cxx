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
  // Starts gathering data.
  LogMessage("Starting workers");

  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    (*it)->StartWorker();
  }
}

void WfdContainer::StartThreads()
{
  // Launches the data worker threads.
  LogMessage("Launching worker threads");

  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    (*it)->StartThread();
  }
}

void WfdContainer::StopWorkers()
{
  // Stop collecting data.
  LogMessage("Stopping workers");

  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    (*it)->StopWorker();
  }
}

void WfdContainer::StopThreads()
{
  // Stop and rejoin worker threads.
  LogMessage("Stopping worker threads");

  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    (*it)->StopThread();
  }
}

bool WfdContainer::AllWorkersHaveEvent()
{
  // Check each worker for an event.
  bool has_event = true;

  // If any worker doesn't have an event, has_event will become false.
  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    has_event &= (*it)->has_event();
  }

  return has_event;
}

bool WfdContainer::AnyWorkersHaveEvent()
{
  // Check each worker for an event.
  bool any_events = false;

  // A bitwise or here works, so that any event will return positive.
  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    any_events |= (*it)->has_event();
  }

  return any_events;
}

bool WfdContainer::AnyWorkersHaveMultiEvent()
{
  // Check each worker for more than one event.
  int num_events = 0;

  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    num_events = (*it)->num_events();
    if (num_events > 1) return true;
  }

  return false;
}

void WfdContainer::GenerateTrigger()
{
  // Launches the data worker threads.
  LogMessage("Issuing software trigger to workers");

  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    (*it)->GenerateTrigger();
  }
}

void WfdContainer::GetEventData(event_data_t &bundle)
{
  // Loops over each worker and collect the event data.
  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    bundle.push_back((*it)->PopEvent());
  }
}

void WfdContainer::FlushEventData()
{
  // Drops any stale events when workers should have no events.
  for (auto it = workers_.begin(); it != workers_.end(); ++it) {
    (*it)->FlushEvents();
  }
}

void WfdContainer::FreeList()
{
  // Delete the allocated workers.
  LogMessage("Freeing workers");

  for (auto it = workers_.begin(); it != workers_.end(); ++it) {       
    delete *it;
  }

  Resize(0);
}

} // ::hw

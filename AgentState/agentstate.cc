#include "agentstate.hh"

using namespace bold;
using namespace std;

void AgentState::registerObserver(shared_ptr<StateObserver> observer)
{
  assert(observer);

  // TODO STATE assert that we are in the configuration phase

  // Store the observer by each type is supports
  for (std::type_index const& type : observer->getTypes())
  {
    auto it = d_observersByTypeIndex.find(type);
    assert(it != d_observersByTypeIndex.end() && "Tracker type must be registered");
    it->second.push_back(observer);
  }

  // Store the observer by ThreadUtil
  auto it2 = d_observersByThreadId.find((int)observer->getCallbackThreadId());
  assert(it2 != d_observersByThreadId.end() && "Observers not supported for this ThreadI");
  it2->second.push_back(observer);
}

void AgentState::callbackObservers(ThreadIds threadId, SequentialTimer& timer) const
{
  // TODO STATE assert that we are NOT in the configuration phase

  // Find all observers for the specified thread and call them, if they're dirty
  auto it = d_observersByThreadId.find((int)threadId);
  assert(it != d_observersByThreadId.end() && "No observers for specified thread");

  for (shared_ptr<StateObserver> const& observer : it->second)
  {
    if (observer->testAndClearDirty())
    {
      timer.enter(observer->getName());
      observer->observe(timer);
      timer.exit();
    }
  }
}

std::shared_ptr<StateObject const> AgentState::getByTypeIndex(std::type_index const& typeIndex)
{
  // TODO STATE if this is readonly at this point, do we need to lock? can trackers be added/removed dynamically? just assert we're not in configuration mode
  lock_guard<mutex> guard(d_mutex);
  auto pair = d_trackerByTypeId.find(typeIndex);
  assert(pair != d_trackerByTypeId.end() && "Tracker type must be registered");
  auto tracker = pair->second;
  return tracker->stateBase();
}

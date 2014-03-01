#include "state.hh"

using namespace bold;
using namespace std;

sigc::signal<void, shared_ptr<StateTracker const>> State::updated;

mutex State::d_mutex;

unordered_map<type_index, vector<shared_ptr<StateObserver>>> State::d_observersByTypeIndex;
unordered_map<int, vector<shared_ptr<StateObserver>>> State::d_observersByThreadId;
unordered_map<type_index, shared_ptr<StateTracker>> State::d_trackerByTypeId;
unordered_map<string, shared_ptr<StateTracker>> State::d_trackerByName;

void State::initialise()
{
  // Only allow observers to be called back on specified threads
  d_observersByThreadId[(int)ThreadId::MotionLoop] = vector<shared_ptr<StateObserver>>();
  d_observersByThreadId[(int)ThreadId::ThinkLoop] = vector<shared_ptr<StateObserver>>();
}

shared_ptr<StateObject const> State::getByName(string name)
{
  auto it = d_trackerByName.find(name);
  if (it == d_trackerByName.end())
  {
    log::warning("State::getByName") << "No tracker exists with name " << name;
    return nullptr;
  }

  shared_ptr<StateTracker> const& tracker = it->second;
  return tracker->stateBase();
}

vector<shared_ptr<StateTracker>> State::getTrackers()
{
  vector<shared_ptr<StateTracker>> stateObjects;
  lock_guard<mutex> guard(d_mutex);
  transform(d_trackerByTypeId.begin(), d_trackerByTypeId.end(),
                  back_inserter(stateObjects),
                  [](decltype(d_trackerByTypeId)::value_type const& pair) { return pair.second; });
  return stateObjects;
}

void State::registerObserver(shared_ptr<StateObserver> observer)
{
  assert(observer);

  // TODO STATE assert that we are in the configuration phase

  // Store the observer by each type is supports
  for (type_index const& type : observer->getTypes())
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

void State::callbackObservers(ThreadId threadId, SequentialTimer& timer)
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

shared_ptr<StateObject const> State::getByTypeIndex(type_index const& typeIndex)
{
  // TODO STATE if this is readonly at this point, do we need to lock? can trackers be added/removed dynamically? just assert we're not in configuration mode
  lock_guard<mutex> guard(d_mutex);
  auto pair = d_trackerByTypeId.find(typeIndex);
  assert(pair != d_trackerByTypeId.end() && "Tracker type must be registered");
  auto tracker = pair->second;
  return tracker->stateBase();
}

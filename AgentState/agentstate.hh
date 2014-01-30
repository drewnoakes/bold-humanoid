#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <sigc++/signal.h>
#include <type_traits>
#include <typeindex>
#include <vector>

#include "../SequentialTimer/sequentialtimer.hh"
#include "../StateObject/stateobject.hh"
#include "../StateObserver/stateobserver.hh"
#include "../util/log.hh"

struct libwebsocket_protocols;

namespace bold
{
  class StateTracker
  {
  public:
    template<typename T>
    static std::shared_ptr<StateTracker> create(std::string name)
    {
      static_assert(std::is_base_of<StateObject, T>::value, "T must be a descendant of StateObject");
      return std::make_shared<StateTracker>(name);
    }

    StateTracker(std::string name)
    : websocketProtocol(nullptr),
      d_name(name)
    {}

    void set(std::shared_ptr<StateObject const> state)
    {
      std::lock_guard<std::mutex> guard(d_mutex);
      d_state = state;
      d_updateCount++;
    }

    template<typename T>
    std::shared_ptr<T const> state() const
    {
      static_assert(std::is_base_of<StateObject, T>::value, "T must be a descendant of StateObject");
      std::lock_guard<std::mutex> guard(d_mutex);
      return std::dynamic_pointer_cast<T const>(d_state);
    }

    std::shared_ptr<StateObject const> stateBase() const
    {
      std::lock_guard<std::mutex> guard(d_mutex);
      return d_state;
    }

    std::string name() const { return d_name; }
    long long unsigned updateCount() const { return d_updateCount; }

    libwebsocket_protocols* websocketProtocol;

  private:
    mutable std::mutex d_mutex;
    const std::string d_name;
    std::shared_ptr<StateObject const> d_state;
    long long unsigned d_updateCount;
  };

  class AgentState
  {
  public:
    static void initialise()
    {
      // Only allow observers to be called back on specified threads
      d_observersByThreadId[(int)ThreadId::MotionLoop] = std::vector<std::shared_ptr<StateObserver>>();
      d_observersByThreadId[(int)ThreadId::ThinkLoop] = std::vector<std::shared_ptr<StateObserver>>();
    }

    template<typename T>
    static void registerStateType(std::string name)
    {
      static_assert(std::is_base_of<StateObject, T>::value, "T must be a descendant of StateObject");
      log::verbose("AgentState::registerStateType") << "Registering state type: " << name;
      std::lock_guard<std::mutex> guard(d_mutex);
      assert(d_trackerByTypeId.find(typeid(T)) == d_trackerByTypeId.end()); // assert that it doesn't exist yet
      d_trackerByTypeId[typeid(T)] = StateTracker::create<T>(name);

      // Create an empty list for the observers so that we don't have to lock later on the observer map
      std::vector<std::shared_ptr<StateObserver>> observers = {};
      d_observersByTypeIndex[typeid(T)] = observers;
    }

    static std::vector<std::shared_ptr<StateTracker>> getTrackers()
    {
      std::vector<std::shared_ptr<StateTracker>> stateObjects;
      std::lock_guard<std::mutex> guard(d_mutex);
      std::transform(d_trackerByTypeId.begin(), d_trackerByTypeId.end(),
                     std::back_inserter(stateObjects),
                     [](decltype(d_trackerByTypeId)::value_type const& pair) { return pair.second; });
      return stateObjects;
    }

    static unsigned stateTypeCount() { return d_trackerByTypeId.size(); }

    /** Fires when a state object is updated.
     *
     * Currently only used by DataStreamer, which may serialise the StateObject
     * for attached clients.
     *
     * A lock is held over d_mutex while this event is raised :(
     *
     * TODO avoid unbounded holding of d_mutex by getting rid of this signal and using a queue for the DataStreamer thread to process (?)
     */
    static sigc::signal<void, std::shared_ptr<StateTracker const>> updated;

    static void registerObserver(std::shared_ptr<StateObserver> observer);

    static void callbackObservers(ThreadId threadId, SequentialTimer& timer);

    template <typename T>
    static void set(std::shared_ptr<T const> state)
    {
      static_assert(std::is_base_of<StateObject, T>::value, "T must be a descendant of StateObject");
      assert(state);

      auto const& tracker = getTracker<T const>();
      tracker->set(state);

      std::vector<std::shared_ptr<StateObserver>>* observers;

      // Raise event and look up observers while holding lock
      {
        // TODO this blocks for too long. eventing won't work well. need to do all updates async, off the motion thread
        std::lock_guard<std::mutex> guard(d_mutex);
        updated(tracker);

        auto it = d_observersByTypeIndex.find(typeid(T));
        assert(it != d_observersByTypeIndex.end());
        observers = &it->second;
      }

      // Release lock before notifying observers
      for (auto& observer : *observers)
      {
        assert(observer);
        observer->setDirty();
      }
    }

    /** Get the StateObject of specified type T. May be nullptr.
     */
    template <typename T>
    static std::shared_ptr<T const> get()
    {
      static_assert(std::is_base_of<StateObject, T>::value, "T must be a descendant of StateObject");
      return AgentState::getTrackerState<T>();
    }

    template<typename T>
    static std::shared_ptr<T const> getTrackerState()
    {
      static_assert(std::is_base_of<StateObject, T>::value, "T must be a descendant of StateObject");
      std::lock_guard<std::mutex> guard(d_mutex);
      auto pair = d_trackerByTypeId.find(typeid(T));
      assert(pair != d_trackerByTypeId.end() && "Tracker type must be registered");
      auto tracker = pair->second;
      return tracker->state<T>();
    }

    static std::shared_ptr<StateObject const> getByTypeIndex(std::type_index const& typeIndex);

    template<typename T>
    static std::shared_ptr<StateTracker> getTracker()
    {
      static_assert(std::is_base_of<StateObject, T>::value, "T must be a descendant of StateObject");
      std::lock_guard<std::mutex> guard(d_mutex);
      auto pair = d_trackerByTypeId.find(typeid(T));
      assert(pair != d_trackerByTypeId.end() && "Tracker type must be registered");
      auto tracker = pair->second;
      return tracker;
    }

  private:
    AgentState() = delete;

    static std::mutex d_mutex;

    static std::unordered_map<std::type_index, std::vector<std::shared_ptr<StateObserver>>> d_observersByTypeIndex;
    static std::unordered_map<int, std::vector<std::shared_ptr<StateObserver>>> d_observersByThreadId;
    static std::unordered_map<std::type_index, std::shared_ptr<StateTracker>> d_trackerByTypeId;
  };
}

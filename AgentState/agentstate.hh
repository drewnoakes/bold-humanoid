#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <sigc++/signal.h>
#include <vector>
#include <mutex>

#include "../StateObject/stateobject.hh"
#include "../StateObserver/stateobserver.hh"

struct libwebsocket_protocols;

namespace bold
{
  class StateTracker
  {
  public:
    template<typename T>
    static std::shared_ptr<StateTracker> create(std::string name)
    {
      return std::make_shared<StateTracker>(name/*, &typeid(T)*/);
    }

    StateTracker(std::string name/*, const std::type_info* typeId*/)
    : d_name(name)
      /*, d_typeid(typeId)*/
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

    libwebsocket_protocols* websocketProtocol;;

  private:
    mutable std::mutex d_mutex;
    const std::string d_name;
    // Not used: const std::type_info* d_typeid;
    std::shared_ptr<StateObject const> d_state;
    long long unsigned d_updateCount;
  };

  class AgentState
  {
  public:
    AgentState() {}
    
    template<typename T>
    void registerStateType(std::string name)
    {
      std::cout << "[AgentState::registerStateType] Registering state type: " << name << std::endl;
      std::lock_guard<std::mutex> guard(d_mutex);
      const std::type_info* typeId = &typeid(T);
      assert(d_trackerByTypeId.find(typeId) == d_trackerByTypeId.end()); // assert that it doesn't exist yet
      d_trackerByTypeId[typeId] = StateTracker::create<T>(name);
    }

    std::vector<std::shared_ptr<StateTracker>> getTrackers() const
    {
      std::vector<std::shared_ptr<StateTracker>> stateObjects;
      std::lock_guard<std::mutex> guard(d_mutex);
      std::transform(d_trackerByTypeId.begin(), d_trackerByTypeId.end(),
                     std::back_inserter(stateObjects),
                     [](decltype(d_trackerByTypeId)::value_type const& pair) { return pair.second; });
      return stateObjects;
    }

    unsigned stateTypeCount() const { return d_trackerByTypeId.size(); }

    // TODO get rid of the updated signal, as it causes updates to occur on the motion thread, when they should be on the think thread, or even another thread
    /** Fires when a state object is updated. */
    sigc::signal<void, std::shared_ptr<StateTracker>> updated;

    template<typename TState>
    void registerObserver(std::shared_ptr<StateObserver> observer)
    {
      // TODO can type traits be used here to guarantee that T derives from StateObject
      std::type_info const* typeId = &typeid(TState);
      assert(observer);
      std::lock_guard<std::mutex> guard(d_mutex);
      auto it = d_observersByTypeId.find(typeId);
      if (it == d_observersByTypeId.end())
      {
        std::vector<std::shared_ptr<StateObserver>> observers = { observer };
        d_observersByTypeId[typeId] = observers;
      }
      else
      {
        it->second.push_back(observer);
      }
    }

    template <typename T>
    void set(std::shared_ptr<T const> state)
    {
      assert(state);
      
      // TODO can type traits be used here to guarantee that T derives from StateObject
      auto const& tracker = getTracker<T const>();
      tracker->set(state);
      
      // TODO this blocks for too long. eventing won't work well. need to do all updates async, off the motion thread
      std::lock_guard<std::mutex> guard(d_mutex);
      updated(tracker);
      
      std::type_info const* typeId = &typeid(T);
      auto it = d_observersByTypeId.find(typeId);
      if (it != d_observersByTypeId.end())
      {
        std::vector<std::shared_ptr<StateObserver>> const& observers = it->second;
        assert(observers.size());
        for (auto& observer : observers)
        {
          assert(observer);
          observer->observe(state);
        }
      }
      // TODO we hold this lock throughout all observers... dodgy!
    }

    /** Get the StateObject of specified type T. May be nullptr.
     */
    template <typename T>
    static std::shared_ptr<T const> get()
    {
      return AgentState::getInstance().getTrackerState<T>();
    }

    template<typename T>
    std::shared_ptr<T const> getTrackerState() const
    {
      std::lock_guard<std::mutex> guard(d_mutex);
      auto pair = d_trackerByTypeId.find(&typeid(T));
      assert(pair != d_trackerByTypeId.end() && "Tracker type must be registered");
      auto tracker = pair->second;
      auto state = tracker->state<T>();
      return state;
    }

    template<typename T>
    std::shared_ptr<StateTracker> getTracker() const
    {
      std::lock_guard<std::mutex> guard(d_mutex);
      auto pair = d_trackerByTypeId.find(&typeid(T));
      assert(pair != d_trackerByTypeId.end() && "Tracker type must be registered");
      auto tracker = pair->second;
      return tracker;
    }

    static AgentState& getInstance();

  private:
    mutable std::mutex d_mutex;

    struct TypeInfoCompare { bool operator()(std::type_info const* a, std::type_info const* b) const { return a->before(*b); }; };

    std::map<std::type_info const*, std::vector<std::shared_ptr<StateObserver>>, TypeInfoCompare> d_observersByTypeId;
    std::map<std::type_info const*, std::shared_ptr<StateTracker>, TypeInfoCompare> d_trackerByTypeId;
  };

  inline AgentState& AgentState::getInstance()
  {
    static AgentState instance;
    return instance;
  }
}

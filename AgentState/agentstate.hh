#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <sigc++/signal.h>
#include <vector>
#include <semaphore.h>

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
      return std::make_shared<StateTracker>(name, &typeid(T));
    }

    StateTracker(std::string name, const std::type_info* typeId)
    : d_name(name),
      d_typeid(typeId)
    {}

    void set(std::shared_ptr<StateObject const> state)
    {
      d_state = state;
      d_updateCount++;
    }

    template<typename T>
    std::shared_ptr<T const> state() const
    {
      return std::dynamic_pointer_cast<T const>(d_state);
    }

    std::shared_ptr<StateObject const> stateBase() const
    {
      return d_state;
    }

    std::string name() const { return d_name; }
    long long unsigned updateCount() const { return d_updateCount; }

    libwebsocket_protocols* websocketProtocol;;

  private:
    const std::string d_name;
    const std::type_info* d_typeid;
    std::shared_ptr<StateObject const> d_state;
    long long unsigned d_updateCount;
  };

  class AgentState
  {
  public:
    AgentState()
    {
      sem_init(&d_lock, 0, 1);
    }
    
    template<typename T>
    void registerStateType(std::string name)
    {
      std::cout << "[AgentState::registerStateType] Registering state type: " << name << std::endl;
      lock();
      const std::type_info* typeId = &typeid(T);
      assert(d_trackerByTypeId.find(typeId) == d_trackerByTypeId.end()); // assert that it doesn't exist yet
      d_trackerByTypeId[typeId] = StateTracker::create<T>(name);
      unlock();
    }

    std::vector<std::shared_ptr<StateTracker>> getTrackers() const
    {
      std::vector<std::shared_ptr<StateTracker>> stateObjects;
      lock();
      std::transform(d_trackerByTypeId.begin(), d_trackerByTypeId.end(),
                     std::back_inserter(stateObjects),
                     [](decltype(d_trackerByTypeId)::value_type const& pair) { return pair.second; });
      unlock();
      return stateObjects;
    }

    unsigned stateTypeCount() const { return d_trackerByTypeId.size(); }

    /** Fires when a state object is updated. */
    sigc::signal<void, std::shared_ptr<StateTracker>> updated;

    template<typename TState>
    void registerObserver(std::shared_ptr<StateObserver> observer)
    {
      // TODO can type traits be used here to guarantee that T derives from StateObject
      std::type_info const* typeId = &typeid(TState);
      assert(observer);
      lock();
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
      unlock();
    }

    template <typename T>
    void set(std::shared_ptr<T const> state)
    {
      assert(state);
      // TODO can type traits be used here to guarantee that T derives from StateObject
      auto const& tracker = getTracker<T const>();
      tracker->set(state);
      updated(tracker);
      
      std::type_info const* typeId = &typeid(T);
      lock();
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
      unlock();
    }

    template <typename T>
    static std::shared_ptr<T const> get()
    {
      std::shared_ptr<StateTracker> tracker = AgentState::getInstance().getTracker<T>();
      return tracker->state<T const>();
    }

    template<typename T>
    std::shared_ptr<StateTracker> getTracker() const
    {
      lock();
      auto pair = d_trackerByTypeId.find(&typeid(T));
      assert(pair != d_trackerByTypeId.end() && "Tracker type must be registered");
      unlock();
      return pair->second;
    }

    static AgentState& getInstance();

  private:
    mutable sem_t d_lock;

    void lock() const { while((sem_wait(&d_lock) == -1) && (errno == EINTR)); }
    void unlock() const { sem_post(&d_lock); }
    
    struct TypeInfoCompare { bool operator()(std::type_info const* a, std::type_info const* b) const { return a->before(*b); }; };

    std::map<std::type_info const*, std::vector<std::shared_ptr<StateObserver>>> d_observersByTypeId;
    std::map<std::type_info const*, std::shared_ptr<StateTracker>, TypeInfoCompare> d_trackerByTypeId;
  };

  inline AgentState& AgentState::getInstance()
  {
    static AgentState instance;
    return instance;
  }
}

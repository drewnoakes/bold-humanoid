#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <sigc++/signal.h>
#include <vector>

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
    {}

    template<typename T>
    void registerStateType(std::string name)
    {
      std::cout << "[AgentState::registerStateType] Registering state type: " << name << std::endl;
      const std::type_info* typeId = &typeid(T);
      assert(d_trackerByTypeId.find(typeId) == d_trackerByTypeId.end()); // assert that it doesn't exist yet
      d_trackerByTypeId[typeId] = StateTracker::create<T>(name);
    }

    std::vector<std::shared_ptr<StateTracker>> getTrackers() const
    {
      std::vector<std::shared_ptr<StateTracker>> stateObjects;
      std::transform(d_trackerByTypeId.begin(), d_trackerByTypeId.end(),
                     std::back_inserter(stateObjects),
                     [](decltype(d_trackerByTypeId)::value_type const& pair) { return pair.second; });
      return stateObjects;
    }

    unsigned stateTypeCount() const { return d_trackerByTypeId.size(); }

    /** Fires when a state object is updated. */
    sigc::signal<void, std::shared_ptr<StateTracker>> updated;

    std::map<std::type_info const*, std::vector<std::shared_ptr<StateObserver<StateObject>>>> d_observersByTypeId;

    template<typename TState>
    void registerObserver(std::shared_ptr<StateObserver<TState>> observer)
    {
      // TODO can type traits be used here to guarantee that T derives from StateObject
      std::type_info const* typeId = &typeid(TState);
      auto it = d_observersByTypeId.find(typeId);
      if (it == d_observersByTypeId.end())
      {
        std::vector<std::shared_ptr<StateObserver<StateObject>>> observers = { std::dynamic_pointer_cast<StateObserver<StateObject>>(observer) };
        d_observersByTypeId[typeId] = observers;
      }
      else
      {
        it->second.push_back(std::dynamic_pointer_cast<StateObserver<StateObject>>(observer));
      }
    }

    template <typename T>
    void set(std::shared_ptr<T const> state)
    {
      // TODO can type traits be used here to guarantee that T derives from StateObject
      auto const& tracker = getTracker<T const>();
      tracker->set(state);
      updated(tracker);
      std::type_info const* typeId = &typeid(T);
      auto it = d_observersByTypeId.find(typeId);
      if (it != d_observersByTypeId.end())
      {
        for (auto& observer : it->second)
        {
          std::dynamic_pointer_cast<StateObserver<T>>(observer)->observe(state);
        }
      }
    }

    template <typename T>
    static std::shared_ptr<T const> get()
    {
      auto const& instance = AgentState::getInstance();
      std::shared_ptr<StateTracker> tracker = instance.getTracker<T>();
      return tracker->state<T const>();
    }

    template<typename T>
    std::shared_ptr<StateTracker> getTracker() const
    {
      auto pair = d_trackerByTypeId.find(&typeid(T));
      assert(pair != d_trackerByTypeId.end() && "Tracker type must be registered");
      return pair->second;
    }

    static AgentState& getInstance();

  private:
    struct TypeInfoCompare { bool operator()(std::type_info const* a, std::type_info const* b) const { return a->before(*b); }; };

    std::map<std::type_info const*, std::shared_ptr<StateTracker>, TypeInfoCompare> d_trackerByTypeId;
  };

  inline AgentState& AgentState::getInstance()
  {
    static AgentState instance;
    return instance;
  }
}

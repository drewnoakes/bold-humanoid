#ifndef BOLD_AGENTSTATE_HH
#define BOLD_AGENTSTATE_HH

#include <cassert>
#include <memory>
#include <vector>
#include <sigc++/signal.h>

#include "../StateObject/stateobject.hh"
#include <map>

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
      const std::type_info* typeId = &typeid(T);
      assert(d_trackerByTypeId.find(typeId) == d_trackerByTypeId.end()); // assert doesn't exist yet
      std::shared_ptr<StateTracker> tracker = StateTracker::create<T>(name);
      d_trackerByTypeId[typeId] = tracker;
    }

    std::vector<std::shared_ptr<StateTracker>> getTrackers() const
    {
      std::vector<std::shared_ptr<StateTracker>> stateObjects;
      for (auto const& pair : d_trackerByTypeId) {
        stateObjects.push_back(pair.second);
      }
      return stateObjects;
    }

    unsigned stateTypeCount() const { return d_trackerByTypeId.size(); }

    /** Fires when a state object is updated. */
    sigc::signal<void, std::shared_ptr<StateTracker>> updated;

    template <typename T>
    void set(std::shared_ptr<T const> state)
    {
      auto const& tracker = getTracker<T const>();
      tracker->set(state);
      updated(tracker);
    }

    template <typename T>
    std::shared_ptr<T const> get() const
    {
      std::shared_ptr<StateTracker> tracker = getTracker<T>();
      return tracker->state<T const>();
    }

    template<typename T>
    std::shared_ptr<StateTracker> getTracker() const
    {
      auto pair = d_trackerByTypeId.find(&typeid(T));
      assert(pair != d_trackerByTypeId.end()); // assert exists
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

#endif

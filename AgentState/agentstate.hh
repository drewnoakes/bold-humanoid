#ifndef BOLD_AGENTSTATE_HH
#define BOLD_AGENTSTATE_HH

#include <memory>
#include <vector>
#include <sigc++/signal.h>

#include "../StateObject/stateobject.hh"
#include <map>

namespace bold
{
  class AgentState
  {
  public:
    AgentState()
    : d_cameraFrameNumber(0)
    {}

    unsigned long long getCameraFrameNumber() const { return d_cameraFrameNumber; }

    std::vector<std::shared_ptr<StateObject>> allStateObjects() const;

    std::vector<StateType> allStateTypes() const
    {
      std::vector<StateType> types = {
        StateType::AgentFrame,
        StateType::Alarm,
        StateType::Body,
        StateType::CameraFrame,
        StateType::Game,
        StateType::Hardware,
        StateType::WorldFrame,
      };
      return types;
    }

    /** Fires when a state object is updated.
     */
    sigc::signal<void, StateType, std::shared_ptr<StateObject>> updated;

    //
    // Setters
    //

    template <typename T>
    void set(std::shared_ptr<T> state)
    {
      // Todo: check whether this is atomic
      d_stateByTypeId[&typeid(T)] = state;
    }

    template <typename T>
    std::shared_ptr<T const> get() const
    {
      // Todo: this definitely is not atomic
      auto s = d_stateByTypeId.find(&typeid(T));
      return s != d_stateByTypeId.end() ? std::static_pointer_cast<T const>(s->second) : 0;
    }

    static AgentState& getInstance();

  private:
    unsigned long d_cameraFrameNumber;

    struct TypeInfoCompare { bool operator()(std::type_info const* a, std::type_info const* b) const { return a->before(*b); }; };

    std::map<std::type_info const*, std::shared_ptr<StateObject>, TypeInfoCompare> d_stateByTypeId;
  };

  inline AgentState& AgentState::getInstance()
  {
    static AgentState instance;
    return instance;
  }
}

#endif

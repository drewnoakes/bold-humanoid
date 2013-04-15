#ifndef BOLD_AGENTSTATE_HH
#define BOLD_AGENTSTATE_HH

#include <memory>
#include <vector>
#include <sigc++/signal.h>

#include "../StateObject/stateobject.hh"
#include <map>

namespace bold
{
  class AgentFrameState;
  class AlarmState;
  class BodyState;
  class CameraFrameState;
  class GameState;
  class HardwareState;
  class WorldFrameState;

  class AgentState
  {
  public:
    AgentState()
    : d_cameraFrameNumber(0),
      d_agentFrame(),
      d_alarmState(),
      d_bodyState(),
      d_cameraFrame(),
      d_gameState(),
      d_hardwareState(),
      d_worldFrameState()
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
      d_states[&typeid(T)] = state;
    }

    template <typename T>
    std::shared_ptr<T const> get() const
    {
      // Todo: this definitely is not atomic
      auto s = d_states.find(&typeid(T));
      return s != d_states.end() ? std::static_pointer_cast<T const>(s->second) : 0;
    }

    static AgentState& getInstance();

  private:
    unsigned long d_cameraFrameNumber;
    std::shared_ptr<AgentFrameState> d_agentFrame;
    std::shared_ptr<AlarmState> d_alarmState;
    std::shared_ptr<BodyState> d_bodyState;
    std::shared_ptr<CameraFrameState> d_cameraFrame;
    std::shared_ptr<GameState> d_gameState;
    std::shared_ptr<HardwareState> d_hardwareState;
    std::shared_ptr<WorldFrameState> d_worldFrameState;

    struct TypeInfoCompare { bool operator()(std::type_info const* a, std::type_info const* b) const { return a->before(*b); }; };

    std::map<std::type_info const*, std::shared_ptr<StateObject>,TypeInfoCompare> d_states;
  };

  inline AgentState& AgentState::getInstance()
  {
    static AgentState instance;
    return instance;
  }
}

#endif

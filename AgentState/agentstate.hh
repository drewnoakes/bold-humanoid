#ifndef BOLD_AGENTSTATE_HH
#define BOLD_AGENTSTATE_HH

#include <memory>
#include <vector>
#include <sigc++/signal.h>

#include "../StateObject/stateobject.hh"

namespace bold
{
  class AgentFrameState;
  class CameraFrameState;
  class GameState;
  class HardwareState;
  class BodyState;
  class AlarmState;

  class AgentState
  {
  public:
    AgentState()
    : d_cameraFrameNumber(0),
      d_cameraFrame(),
      d_agentFrame(),
      d_gameState(),
      d_hardwareState(),
      d_bodyState(),
      d_alarmState()
    {}

    std::shared_ptr<CameraFrameState> cameraFrame() const { return d_cameraFrame; }
    std::shared_ptr<AgentFrameState> agentFrame() const { return d_agentFrame; }
    std::shared_ptr<GameState> game() const { return d_gameState; }
    std::shared_ptr<HardwareState> hardware() const { return d_hardwareState; }
    std::shared_ptr<BodyState> body() const { return d_bodyState; }
    std::shared_ptr<AlarmState> alarm() const { return d_alarmState; }

    /** Fires when a state object is updated.
     */
    sigc::signal<void, StateType, std::shared_ptr<StateObject>> updated;

    sigc::signal<void> gameUpdated;
    sigc::signal<void> bodyUpdated;

    std::vector<std::shared_ptr<StateObject>> allStateObjects() const;

    unsigned long long getCameraFrameNumber() const { return d_cameraFrameNumber; }

    //
    // Setters
    //

    void setCameraFrame(std::shared_ptr<CameraFrameState> cameraFrame);
    void setAgentFrame(std::shared_ptr<AgentFrameState> agentFrame);
    void setGameState(std::shared_ptr<GameState> const& gameState);
    void setHardwareState(std::shared_ptr<HardwareState> hardwareState);
    void setBodyState(std::shared_ptr<BodyState> bodyState);
    void setAlarmState(std::shared_ptr<AlarmState> alarmState);

    static AgentState& getInstance();

  private:
    unsigned long d_cameraFrameNumber;
    std::shared_ptr<CameraFrameState> d_cameraFrame;
    std::shared_ptr<AgentFrameState> d_agentFrame;
    std::shared_ptr<GameState> d_gameState;
    std::shared_ptr<HardwareState> d_hardwareState;
    std::shared_ptr<BodyState> d_bodyState;
    std::shared_ptr<AlarmState> d_alarmState;
  };

  inline AgentState& AgentState::getInstance()
  {
    static AgentState instance;
    return instance;
  }
}

#endif

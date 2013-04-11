#ifndef BOLD_AGENTSTATE_HH
#define BOLD_AGENTSTATE_HH

#include <memory>

namespace bold
{
  class AgentFrameState;
  class CameraFrameState;
  class GameState;
  class HardwareState;
  class BodyState;

  class AgentState
  {
  public:
    AgentState()
    : d_cameraFrameNumber(0)
    {};

    std::shared_ptr<CameraFrameState> cameraFrame() const { return d_cameraFrame; }
    std::shared_ptr<AgentFrameState> agentFrame() const { return d_agentFrame; }
    std::shared_ptr<GameState> game() const { return d_gameState; }
    std::shared_ptr<HardwareState> hardware() const { return d_hardwareState; }
    std::shared_ptr<BodyState> body() const { return d_bodyState; }

    unsigned long long getCameraFrameNumber() const { return d_cameraFrameNumber; }

    //
    // Setters
    //

    void setAgentFrame(std::shared_ptr<AgentFrameState> agentFrame)
    {
      d_agentFrame = agentFrame;
    }

    void setCameraFrame(std::shared_ptr<CameraFrameState> cameraFrame)
    {
      d_cameraFrameNumber++;
      d_cameraFrame = cameraFrame;
    }

    void setGameState(std::shared_ptr<GameState> const& gameState)
    {
      d_gameState = gameState;
    }

    void setHardwareState(std::shared_ptr<HardwareState>& hardwareState)
    {
      d_hardwareState = hardwareState;
    }

    static AgentState& getInstance();

  private:
    unsigned long d_cameraFrameNumber;
    std::shared_ptr<CameraFrameState> d_cameraFrame;
    std::shared_ptr<AgentFrameState> d_agentFrame;
    std::shared_ptr<GameState> d_gameState;
    std::shared_ptr<HardwareState> d_hardwareState;
    std::shared_ptr<BodyState> d_bodyState;
  };

  inline AgentState& AgentState::getInstance()
  {
    static AgentState instance;
    return instance;
  }
}

#endif

#pragma once

#include "../typedstateobserver.hh"

namespace bold
{
  class Voice;

  enum class FallState
  {
    STANDUP,
    BACKWARD,
    FORWARD,
    LEFT,
    RIGHT
  };

  enum class FallDetectorTechnique
  {
    Accelerometer = 1,
    Orientation = 2
  };

  std::string getFallStateName(FallState fallState);

  class FallDetector
  {
  public:
    FallState getFallenState() const { return d_fallenState; }

  protected:
    FallDetector(std::shared_ptr<Voice> voice);

    ~FallDetector() = default;

    void setFallState(FallState fallState);

    virtual void logFallData(std::stringstream& msg) const = 0;

  private:

    std::shared_ptr<Voice> d_voice;
    FallState d_fallenState;
    Clock::Timestamp d_startTime;
  };
}

#ifndef BOLD_AGENT_MODEL_HH
#define BOLD_AGENT_MODEL_HH

#include <Eigen/Core>
#include <sigc++/sigc++.h>
#include "../Agent/agent.hh"
#include "../CM730Snapshot/CM730Snapshot.hh"
#include "../MX28Snapshot/MX28Snapshot.hh"

namespace bold
{
  class AgentModel
  {
  private:
    AgentModel()
    : lastThinkCycleMillis(0),
      lastImageCaptureTimeMillis(0),
      lastImageProcessTimeMillis(0)
//       gyroReading(Eigen::Vector3d(0,0,0)),
//       accelerometerReading(Eigen::Vector3d(0,0,0))
    {};

    AgentModel(AgentModel const&);
    void operator=(AgentModel const&);

  public:

    double lastThinkCycleMillis;
    double lastImageCaptureTimeMillis;
    double lastImageProcessTimeMillis;

//     Eigen::Vector3d gyroReading;
//     Eigen::Vector3d accelerometerReading;

    CM730Snapshot cm730State;
    MX28Snapshot mx28States[JointData::NUMBER_OF_JOINTS];

    std::string state;

    sigc::signal<void> updated;

//     void update(Eigen::Vector3d const&  gyro, Eigen::Vector3d const& accelerometer)
//     {
//       gyroReading = gyro;
//       accelerometerReading = accelerometer;
//       updated();
//     }
//
//     void updateState(bold::State state)
//     {
//       //this->state = bold::State::getName(state);
//     }

    static AgentModel& getInstance()
    {
      static AgentModel instance;
      return instance;
    }
  };
}

#endif
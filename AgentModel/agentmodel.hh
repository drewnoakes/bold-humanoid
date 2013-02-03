#ifndef BOLD_AGENT_MODEL_HH
#define BOLD_AGENT_MODEL_HH

#include <Eigen/Core>
#include <sigc++/sigc++.h>

namespace bold
{
  class AgentModel
  {
  private:
    AgentModel()
    : lastThinkCycleMillis(0),
      lastImageCaptureTimeMillis(0),
      lastImageProcessTimeMillis(0),
      gyroReading(Eigen::Vector3d(0,0,0)),
      accelerometerReading(Eigen::Vector3d(0,0,0))
    {};

    AgentModel(AgentModel const&);
    void operator=(AgentModel const&);

  public:

    double lastThinkCycleMillis;
    double lastImageCaptureTimeMillis;
    double lastImageProcessTimeMillis;

    Eigen::Vector3d gyroReading;
    Eigen::Vector3d accelerometerReading;

    sigc::signal<void> cm730Updated;

    void updateCM730Data(Eigen::Vector3d const&  gyro, Eigen::Vector3d const& accelerometer)
    {
      gyroReading = gyro;
      accelerometerReading = accelerometer;
      cm730Updated();
    }

    static AgentModel& getInstance()
    {
      static AgentModel instance;
      return instance;
    }
  };
}

#endif
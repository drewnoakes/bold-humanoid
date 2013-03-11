#ifndef BOLD_AGENT_MODEL_HH
#define BOLD_AGENT_MODEL_HH

#include <Eigen/Core>
#include <sigc++/sigc++.h>

#include "../robotis/Framework/include/JointData.h"

#include "../Agent/agent.hh"
#include "../CM730Snapshot/CM730Snapshot.hh"
#include "../MX28Snapshot/MX28Snapshot.hh"
#include "../vision/CameraModel/cameramodel.hh"

namespace bold
{
  class AgentModel
  {
  public:
    CM730Snapshot cm730State;
    MX28Snapshot mx28States[Robot::JointData::NUMBER_OF_JOINTS];

    std::string state;

    sigc::signal<void> updated;

    void initialise(minIni const& ini)
    {
      // TODO source imageWidth, imageHeight, focalLength, rangeVertical, rangeHorizontal from config
      int imageWidth = 320;
      int imageHeight = 240;
      double focalLength = 0.025;
      double rangeVertical = 46/180.0 * M_PI;
      double rangeHorizontal = 58/180.0 * M_PI;;

      d_cameraModel = new CameraModel(imageWidth, imageHeight, focalLength, rangeVertical, rangeHorizontal);
    }

    void notifyCycleStarting()
    {
      d_cycleNumber++;
    }

    unsigned long long getCycleNumber() const { return d_cycleNumber; }

    CameraModel& getCameraModel() const { return *d_cameraModel; }

    static AgentModel& getInstance()
    {
      static AgentModel instance;
      return instance;
    }

  private:
    unsigned long long d_cycleNumber;
    CameraModel* d_cameraModel;

    AgentModel()
    : d_cycleNumber(0)
    {};

    AgentModel(AgentModel const&) = delete;
    void operator=(AgentModel const&) = delete;
  };
}

#endif
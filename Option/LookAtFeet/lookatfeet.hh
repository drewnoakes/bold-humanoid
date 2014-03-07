#pragma once

#include "../option.hh"
#include "../../Config/config.hh"
#include "../../stats/movingaverage.hh"
#include "../../util/log.hh"

#include <Eigen/Core>

namespace bold
{
  class HeadModule;

  class LookAtFeet : public Option
  {
  public:
    LookAtFeet(std::string const& id, std::shared_ptr<HeadModule> headModule)
    : Option(id, "LookAtFeet"),
      d_avgBallPos(30),
      d_headModule(headModule)
    {
      d_panDegs = Config::getSetting<double>("options.look-at-feet.head-pan-degs");
      d_tiltDegs = Config::getSetting<double>("options.look-at-feet.head-tilt-degs");
    }

    virtual void reset() override { d_avgBallPos.reset(); }

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    bool hasPosition() const { return d_avgBallPos.count() != 0; }

    Eigen::Vector3d getAverageBallPositionAgentFrame() const
    {
      if (d_avgBallPos.count() == 0)
      {
        log::error("LookAtFeet::getBallPositionAgentFrame") << "No ball observations available";
        throw std::runtime_error("No ball observations available");
      }

      return d_avgBallPos.getAverage();
    }

  private:
    MovingAverage<Eigen::Vector3d> d_avgBallPos;
    std::shared_ptr<HeadModule> d_headModule;
    Setting<double>* d_panDegs;
    Setting<double>* d_tiltDegs;
  };
}

#pragma once

#include "../option.hh"
#include "../../stats/movingaverage.hh"

#include <Eigen/Core>

namespace bold
{
  class HeadModule;
  template<typename> class Setting;

  class LookAtFeet : public Option
  {
  public:
    LookAtFeet(std::string const& id, std::shared_ptr<HeadModule> headModule);

    void reset() override { d_avgBallPos.reset(); }

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    bool hasPosition() const { return d_avgBallPos.count() != 0; }

    Eigen::Vector3d getAverageBallPositionAgentFrame() const;

  private:
    MovingAverage<Eigen::Vector3d> d_avgBallPos;
    std::shared_ptr<HeadModule> d_headModule;
    Setting<double>* d_panDegs;
    Setting<double>* d_tiltDegs;
  };
}

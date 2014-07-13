#pragma once

#include "../option.hh"
#include "../../stats/movingaverage.hh"

#include <Eigen/Core>

namespace bold
{
  class TrackBall : public Option
  {
  public:
    TrackBall(std::string id);

    void reset() override;

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

  private:
    MovingAverage<Eigen::Vector2d> d_slowAverage;
    MovingAverage<Eigen::Vector2d> d_fastAverage;
  };
}

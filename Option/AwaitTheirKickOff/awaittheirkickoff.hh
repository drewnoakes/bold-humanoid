#pragma once

#include "../option.hh"
#include "../../stats/movingaverage.hh"
#include "../../util/Maybe.hh"

#include <Eigen/Core>

namespace bold
{
  template<typename> class Setting;

  /** Terminates only when the opponent's kick-off time elapses, or the ball is moved away from its initial position. */
  class AwaitTheirKickOff : public Option
  {
  public:
    AwaitTheirKickOff(std::string id);

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    double hasTerminated() override;

    void reset() override;

  private:
    /// Smoothed ball position estimate
    MovingAverage<Eigen::Vector2d> d_ballPosition;
    /// Mature estimate of the ball's initial position, against which later mature estimates will be compared
    Maybe<Eigen::Vector2d> d_ballInitialPosition;
    /// Setting that specifies the distance the ball must move in order to terminate this option
    Setting<double>* d_requiredMoveDistance;
  };
}

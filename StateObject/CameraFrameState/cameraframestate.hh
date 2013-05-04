#pragma once

#include <Eigen/Core>

#include <vector>

#include "../stateobject.hh"
#include "../util/Maybe.hh"
#include "../geometry/LineSegment2i.hh"

namespace bold
{
  class CameraFrameState : public StateObject
  {
  public:
    CameraFrameState(Maybe<Eigen::Vector2d> ballObservation,
                     std::vector<Eigen::Vector2d> goalObservations,
                     std::vector<LineSegment2i> observedLineSegments)
    : d_ballObservation(ballObservation),
      d_goalObservations(goalObservations),
      d_observedLineSegments(observedLineSegments)
    {}

    Maybe<Eigen::Vector2d> getBallObservation() const { return d_ballObservation; }
    std::vector<Eigen::Vector2d> getGoalObservations() const { return d_goalObservations; }
    std::vector<LineSegment2i> getObservedLineSegments() const { return d_observedLineSegments; }

    bool isBallVisible() const { return d_ballObservation.hasValue(); }

    virtual void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Maybe<Eigen::Vector2d> d_ballObservation;
    std::vector<Eigen::Vector2d> d_goalObservations;
    std::vector<LineSegment2i> d_observedLineSegments;
  };
}

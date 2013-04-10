#ifndef BOLD_CAMERAFRAMESTATE_HH
#define BOLD_CAMERAFRAMESTATE_HH

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
    CameraFrameState(std::vector<Eigen::Vector2f> goalObservations,
                Maybe<Eigen::Vector2f> ballObservation,
                std::vector<LineSegment2i> observedLineSegments)
    : d_goalObservations(goalObservations),
      d_ballObservation(ballObservation),
      d_observedLineSegments(observedLineSegments)
    {}

    std::vector<Eigen::Vector2f> getGoalObservations() const { return d_goalObservations; }
    Maybe<Eigen::Vector2f> getBallObservation() const { return d_ballObservation; }
    std::vector<LineSegment2i> getObservedLineSegments() const { return d_observedLineSegments; }

    bool isBallVisible() const { return d_ballObservation.hasValue(); }

  private:
    std::vector<Eigen::Vector2f> d_goalObservations;
    Maybe<Eigen::Vector2f> d_ballObservation;
    std::vector<LineSegment2i> d_observedLineSegments;
  };
}

#endif
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
    CameraFrameState(Maybe<Eigen::Vector2f> ballObservation,
                     std::vector<Eigen::Vector2f> goalObservations,
                     std::vector<LineSegment2i> observedLineSegments)
    : StateObject("CameraFrame"),
      d_ballObservation(ballObservation),
      d_goalObservations(goalObservations),
      d_observedLineSegments(observedLineSegments)
    {}

    Maybe<Eigen::Vector2f> getBallObservation() const { return d_ballObservation; }
    std::vector<Eigen::Vector2f> getGoalObservations() const { return d_goalObservations; }
    std::vector<LineSegment2i> getObservedLineSegments() const { return d_observedLineSegments; }

    bool isBallVisible() const { return d_ballObservation.hasValue(); }

    virtual void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Maybe<Eigen::Vector2f> d_ballObservation;
    std::vector<Eigen::Vector2f> d_goalObservations;
    std::vector<LineSegment2i> d_observedLineSegments;
  };
}

#endif
#ifndef BOLD_WORLDFRAMESTATE_HH
#define BOLD_WORLDFRAMESTATE_HH

#include <Eigen/Core>

#include <vector>

#include "../stateobject.hh"
#include "../util/Maybe.hh"
#include "../geometry/LineSegment2i.hh"

namespace bold
{
  class WorldFrameState : public StateObject
  {
  public:
    WorldFrameState(Maybe<Eigen::Vector3d> ballObservation,
                    std::vector<Eigen::Vector3d> goalObservations,
                    std::vector<LineSegment3d> observedLineSegments)
    : StateObject("WorldFrame"),
      d_ballObservation(ballObservation),
      d_goalObservations(goalObservations),
      d_observedLineSegments(observedLineSegments)
    {}

    Maybe<Eigen::Vector3d> getBallObservation() const { return d_ballObservation; }
    std::vector<Eigen::Vector3d> getGoalObservations() const { return d_goalObservations; }
    std::vector<LineSegment3d> getObservedLineSegments() const { return d_observedLineSegments; }

    bool isBallVisible() const { return d_ballObservation.hasValue(); }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Maybe<Eigen::Vector3d> d_ballObservation;
    std::vector<Eigen::Vector3d> d_goalObservations;
    std::vector<LineSegment3d> d_observedLineSegments;
  };
}

#endif
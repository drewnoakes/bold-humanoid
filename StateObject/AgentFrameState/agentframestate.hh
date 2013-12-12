#pragma once

#include <Eigen/Core>

#include <vector>

#include "../stateobject.hh"
#include "../util/Maybe.hh"
#include "../geometry/LineSegment.hh"

namespace bold
{
  class AgentFrameState : public StateObject
  {
  public:
    AgentFrameState(Maybe<Eigen::Vector3d> ballObservation,
                    std::vector<Eigen::Vector3d> goalObservations,
                    std::vector<LineSegment3d> observedLineSegments,
                    std::vector<Eigen::Vector3d> visibleFieldPoly)
    : d_ballObservation(ballObservation),
      d_goalObservations(goalObservations),
      d_observedLineSegments(observedLineSegments),
      d_visibleFieldPoly(visibleFieldPoly)
    {}

    Maybe<Eigen::Vector3d> getBallObservation() const { return d_ballObservation; }
    std::vector<Eigen::Vector3d> getGoalObservations() const { return d_goalObservations; }
    std::vector<LineSegment3d> getObservedLineSegments() const { return d_observedLineSegments; }
    std::vector<Eigen::Vector3d> getVisibleFieldPoly() const { return d_visibleFieldPoly; }

    bool isBallVisible() const { return d_ballObservation.hasValue(); }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Maybe<Eigen::Vector3d> d_ballObservation;
    std::vector<Eigen::Vector3d> d_goalObservations;
    std::vector<LineSegment3d> d_observedLineSegments;
    std::vector<Eigen::Vector3d> d_visibleFieldPoly;
  };
}

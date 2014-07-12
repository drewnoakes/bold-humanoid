#pragma once

#include <Eigen/Core>

#include <vector>

#include "../stateobject.hh"
#include "../../AgentPosition/agentposition.hh"
#include "../../OcclusionRay/occlusionray.hh"
#include "../../geometry/LineSegment/LineSegment2/LineSegment2i/linesegment2i.hh"
#include "../../geometry/Polygon2.hh"
#include "../../util/Maybe.hh"

namespace bold
{
  class WorldFrameState : public StateObject
  {
  public:
    WorldFrameState(Maybe<Eigen::Vector3d> ballObservation,
                    std::vector<Eigen::Vector3d> goalObservations,
                    std::vector<LineSegment3d> observedLineSegments,
                    Maybe<Polygon2d> visibleFieldPoly,
                    std::vector<OcclusionRay<double>> occlusionRays,
                    AgentPosition position)
    : d_ballObservation(ballObservation),
      d_goalObservations(goalObservations),
      d_observedLineSegments(observedLineSegments),
      d_visibleFieldPoly(visibleFieldPoly),
      d_occlusionRays(occlusionRays),
      d_position(position)
    {}

    Maybe<Eigen::Vector3d> getBallObservation() const { return d_ballObservation; }
    std::vector<Eigen::Vector3d> getGoalObservations() const { return d_goalObservations; }
    std::vector<LineSegment3d> getObservedLineSegments() const { return d_observedLineSegments; }
    Maybe<Polygon2d> getVisibleFieldPoly() const { return d_visibleFieldPoly; }
    std::vector<OcclusionRay<double>> getOcclusionRays() const { return d_occlusionRays; }

    AgentPosition getPosition() const { return d_position; }

    bool isBallVisible() const { return d_ballObservation.hasValue(); }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Maybe<Eigen::Vector3d> d_ballObservation;
    std::vector<Eigen::Vector3d> d_goalObservations;
    std::vector<LineSegment3d> d_observedLineSegments;
    Maybe<Polygon2d> d_visibleFieldPoly;
    std::vector<OcclusionRay<double>> d_occlusionRays;
    AgentPosition d_position;
  };
}

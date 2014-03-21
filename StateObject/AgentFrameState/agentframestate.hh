#pragma once

#include <Eigen/Core>

#include <vector>

#include "../stateobject.hh"
#include "../../util/Maybe.hh"
#include "../../geometry/LineSegment/linesegment.hh"
#include "../../geometry/Polygon2.hh"
#include "../../LineJunctionFinder/linejunctionfinder.hh"

namespace bold
{
  class AgentFrameState : public StateObject
  {
  public:
    AgentFrameState(Maybe<Eigen::Vector3d> ballObservation,
                    std::vector<Eigen::Vector3d> goalObservations,
                    std::vector<LineSegment3d> observedLineSegments,
                    std::vector<LineJunction, Eigen::aligned_allocator<LineJunction>> observedLineJunctions,
                    Maybe<Polygon2d> visibleFieldPoly,
                    std::vector<std::pair<Eigen::Vector3d,Eigen::Vector3d>> occlusionRays,
                    ulong thinkCycleNumber)
    : d_ballObservation(ballObservation),
      d_goalObservations(goalObservations),
      d_observedLineSegments(observedLineSegments),
      d_visibleFieldPoly(visibleFieldPoly),
      d_occlusionRays(occlusionRays),
      d_thinkCycleNumber(thinkCycleNumber)
    {}

    Maybe<Eigen::Vector3d> getBallObservation() const { return d_ballObservation; }
    std::vector<Eigen::Vector3d> getGoalObservations() const { return d_goalObservations; }
    std::vector<LineSegment3d> getObservedLineSegments() const { return d_observedLineSegments; }
    Maybe<Polygon2d> getVisibleFieldPoly() const { return d_visibleFieldPoly; }
    std::vector<std::pair<Eigen::Vector3d,Eigen::Vector3d>> getOcclusionRays() const { return d_occlusionRays; }

    bool isBallVisible() const { return d_ballObservation.hasValue(); }
    ulong getThinkCycleNumber() const { return d_thinkCycleNumber; }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Maybe<Eigen::Vector3d> d_ballObservation;
    std::vector<Eigen::Vector3d> d_goalObservations;
    std::vector<LineSegment3d> d_observedLineSegments;
    std::vector<LineJunction, Eigen::aligned_allocator<LineJunction>> d_observerdLineJunctions;
    Maybe<Polygon2d> d_visibleFieldPoly;
    std::vector<std::pair<Eigen::Vector3d,Eigen::Vector3d>> d_occlusionRays;
    ulong d_thinkCycleNumber;
  };
}

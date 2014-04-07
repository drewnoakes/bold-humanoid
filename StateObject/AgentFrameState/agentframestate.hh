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
      : d_ballObservation(std::move(ballObservation)),
      d_goalObservations(std::move(goalObservations)),
      d_observedLineSegments(std::move(observedLineSegments)),
      d_observedLineJunctions(std::move(observedLineJunctions)),
      d_visibleFieldPoly(std::move(visibleFieldPoly)),
      d_occlusionRays(std::move(occlusionRays)),
      d_thinkCycleNumber(thinkCycleNumber)
    {}

    Maybe<Eigen::Vector3d> getBallObservation() const { return d_ballObservation; }
    std::vector<Eigen::Vector3d> getGoalObservations() const { return d_goalObservations; }
    std::vector<LineSegment3d> getObservedLineSegments() const { return d_observedLineSegments; }
    std::vector<LineJunction, Eigen::aligned_allocator<LineJunction>> getObservedLineJunctions() const { return d_observedLineJunctions; }
    Maybe<Polygon2d> getVisibleFieldPoly() const { return d_visibleFieldPoly; }
    std::vector<std::pair<Eigen::Vector3d,Eigen::Vector3d>> getOcclusionRays() const { return d_occlusionRays; }

    bool isBallVisible() const { return d_ballObservation.hasValue(); }
    ulong getThinkCycleNumber() const { return d_thinkCycleNumber; }

    uint goalObservationCount() const { return d_goalObservations.size(); }

    Maybe<Eigen::Vector3d> getClosestGoalObservation() const
    {
      if (d_goalObservations.empty())
        return Maybe<Eigen::Vector3d>::empty();

      auto closestGoalDist = std::numeric_limits<double>::max();
      Maybe<Eigen::Vector3d> closest;

      for (auto const& obs : d_goalObservations)
      {
        auto dist = obs.head<2>().norm();
        if (dist < closestGoalDist)
        {
          closestGoalDist = dist;
          closest = obs;
        }
      }

      return closest;
    }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Maybe<Eigen::Vector3d> d_ballObservation;
    std::vector<Eigen::Vector3d> d_goalObservations;
    std::vector<LineSegment3d> d_observedLineSegments;
    std::vector<LineJunction, Eigen::aligned_allocator<LineJunction>> d_observedLineJunctions;
    Maybe<Polygon2d> d_visibleFieldPoly;
    std::vector<std::pair<Eigen::Vector3d,Eigen::Vector3d>> d_occlusionRays;
    ulong d_thinkCycleNumber;
  };
}

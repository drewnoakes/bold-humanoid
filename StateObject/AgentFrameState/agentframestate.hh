#pragma once

#include <Eigen/Core>

#include <vector>

#include "../stateobject.hh"
#include "../../util/Maybe.hh"
#include "../../geometry/LineSegment/linesegment.hh"
#include "../../geometry/Polygon2.hh"
#include "../../LineJunctionFinder/linejunctionfinder.hh"
#include "../../OcclusionRay/occlusionray.hh"

namespace bold
{
  class AgentFrameState : public StateObject
  {
  public:
    AgentFrameState(Maybe<Eigen::Vector3d> ballObservation,
                    std::vector<Eigen::Vector3d> goalObservations,
                    std::vector<Eigen::Vector3d> teamMateObservations,
                    std::vector<LineSegment3d> observedLineSegments,
                    std::vector<LineJunction, Eigen::aligned_allocator<LineJunction>> observedLineJunctions,
                    Maybe<Polygon2d> visibleFieldPoly,
                    std::vector<OcclusionRay<double>> occlusionRays,
                    ulong thinkCycleNumber);

    Maybe<Eigen::Vector3d> getBallObservation() const { return d_ballObservation; }
    std::vector<Eigen::Vector3d> getGoalObservations() const { return d_goalObservations; }
    std::vector<Eigen::Vector3d> getTeamMateObservations() const { return d_teamMateObservations; }
    std::vector<LineSegment3d> getObservedLineSegments() const { return d_observedLineSegments; }
    std::vector<LineJunction, Eigen::aligned_allocator<LineJunction>> getObservedLineJunctions() const { return d_observedLineJunctions; }
    Maybe<Polygon2d> getVisibleFieldPoly() const { return d_visibleFieldPoly; }
    std::vector<OcclusionRay<double>> getOcclusionRays() const { return d_occlusionRays; }

    static Maybe<Polygon2d> getOcclusionPoly(std::vector<OcclusionRay<double>> const& occlusionRays);
    Maybe<Polygon2d> getOcclusionPoly() const;

    bool isBallVisible() const { return d_ballObservation.hasValue(); }
    ulong getThinkCycleNumber() const { return d_thinkCycleNumber; }

    uint goalObservationCount() const { return d_goalObservations.size(); }

    Maybe<Eigen::Vector3d> getClosestGoalObservation() const;

    double getOcclusionDistance(double angle) const;

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }

    bool shouldSeeAgentFrameGroundPoint(Eigen::Vector2d groundAgent) const;

    bool isNearBall(Eigen::Vector2d point, double maxDistance) const;
    bool isNearGoal(Eigen::Vector2d point, double maxDistance) const;

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer>& writer) const;

    Maybe<Eigen::Vector3d> d_ballObservation;
    std::vector<Eigen::Vector3d> d_goalObservations;
    std::vector<Eigen::Vector3d> d_teamMateObservations;
    std::vector<LineSegment3d> d_observedLineSegments;
    std::vector<LineJunction, Eigen::aligned_allocator<LineJunction>> d_observedLineJunctions;
    Maybe<Polygon2d> d_visibleFieldPoly;
    std::vector<OcclusionRay<double>> d_occlusionRays;
    ulong d_thinkCycleNumber;
  };

  template<typename TBuffer>
  inline void AgentFrameState::writeJsonInternal(rapidjson::Writer<TBuffer>& writer) const
  {
    writer.StartObject();
    {
      writer.String("thinkCycle");
      writer.Uint64(d_thinkCycleNumber);

      writer.String("ball");
      if (d_ballObservation.hasValue())
      {
        writer.StartArray();
        writer.Double(d_ballObservation->x(), "%.3f");
        writer.Double(d_ballObservation->y(), "%.3f");
        writer.Double(d_ballObservation->z(), "%.3f");
        writer.EndArray();
      }
      else
      {
        writer.Null();
      }

      writer.String("goals");
      writer.StartArray();
      {
        for (auto const& goalPos : d_goalObservations)
        {
          writer.StartArray();
          writer.Double(goalPos.x(), "%.3f");
          writer.Double(goalPos.y(), "%.3f");
          writer.Double(goalPos.z(), "%.3f");
          writer.EndArray();
        }
      }
      writer.EndArray();

      writer.String("teammates");
      writer.StartArray();
      {
        for (auto const& teamMatePos : d_teamMateObservations)
        {
          writer.StartArray();
          writer.Double(teamMatePos.x(), "%.3f");
          writer.Double(teamMatePos.y(), "%.3f");
          writer.Double(teamMatePos.z(), "%.3f");
          writer.EndArray();
        }
      }
      writer.EndArray();

      writer.String("lines");
      writer.StartArray();
      {
        for (auto const& lineSeg : d_observedLineSegments)
        {
          writer.StartArray();
          writer.Double(lineSeg.p1().x(), "%.3f");
          writer.Double(lineSeg.p1().y(), "%.3f");
          writer.Double(lineSeg.p1().z(), "%.3f");
          writer.Double(lineSeg.p2().x(), "%.3f");
          writer.Double(lineSeg.p2().y(), "%.3f");
          writer.Double(lineSeg.p2().z(), "%.3f");
          writer.EndArray();
        }
      }
      writer.EndArray();

      writer.String("junctions");
      writer.StartArray();
      {
        for (auto const& junction : d_observedLineJunctions)
        {
          writer.StartObject();
          writer.String("p");
          writer.StartArray();
          writer.Double(junction.position(0), "%.3f");
          writer.Double(junction.position(1), "%.3f");
          writer.EndArray();
          writer.String("a");
          writer.Double(junction.angle, "%.3f");
          writer.String("t");
          writer.Uint(static_cast<unsigned>(junction.type));
          writer.EndObject();
        }
      }
      writer.EndArray();

      writer.String("visibleFieldPoly");
      writer.StartArray();
      {
        if (d_visibleFieldPoly.hasValue())
        {
          for (auto const& vertex : d_visibleFieldPoly.value())
          {
            writer.StartArray();
            writer.Double(vertex.x(), "%.3f");
            writer.Double(vertex.y(), "%.3f");
            writer.EndArray();
          }
        }
      }
      writer.EndArray();

      writer.String("occlusionRays");
      writer.StartArray();
      {
        for (auto const& ray : d_occlusionRays)
        {
          // Should be enough to check a single value for NaN
          if (std::isnan(ray.far().x()))
            continue;

          writer.StartArray();
          writer.Double(ray.near().x(), "%.3f");
          writer.Double(ray.near().y(), "%.3f");
          writer.Double(ray.far().x(), "%.3f");
          writer.Double(ray.far().y(), "%.3f");
          writer.EndArray();
        }
      }
      writer.EndArray();
    }
    writer.EndObject();
  }
}

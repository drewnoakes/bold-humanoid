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

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    Maybe<Eigen::Vector3d> d_ballObservation;
    std::vector<Eigen::Vector3d> d_goalObservations;
    std::vector<LineSegment3d> d_observedLineSegments;
    Maybe<Polygon2d> d_visibleFieldPoly;
    std::vector<OcclusionRay<double>> d_occlusionRays;
    AgentPosition d_position;
  };

  template<typename TBuffer>
  inline void WorldFrameState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("pos");
      writer.StartArray();
      {
        writer.Double(d_position.x(), "%.3f");
        writer.Double(d_position.y(), "%.3f");
        writer.Double(d_position.theta(), "%.4f");
      }
      writer.EndArray();

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

      writer.String("lines");
      writer.StartArray();
      {
        for (LineSegment3d const& lineSeg : d_observedLineSegments)
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

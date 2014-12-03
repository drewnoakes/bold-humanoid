#pragma once

#include <Eigen/Core>
#include <Eigen/StdVector>
#include <vector>

#include "../stateobject.hh"
#include "../../OcclusionRay/occlusionray.hh"
#include "../../geometry/LineSegment/LineSegment2/LineSegment2i/linesegment2i.hh"
#include "../../util/Maybe.hh"

namespace bold
{
  typedef std::vector<Eigen::Vector2d,Eigen::aligned_allocator<Eigen::Vector2d>> Vector2dVector;

  class CameraFrameState : public StateObject
  {
  public:
    CameraFrameState(Maybe<Eigen::Vector2d> ballObservation,
                     Vector2dVector goalObservations,
                     Vector2dVector teamMateObservations,
                     std::vector<LineSegment2i> observedLineSegments,
                     std::vector<OcclusionRay<ushort>> occlusionRays,
                     long totalPixelCount, long processedPixelCount,
                     ulong thinkCycleNumber)
    : d_ballObservation(ballObservation),
      d_goalObservations(goalObservations),
      d_teamMateObservations(teamMateObservations),
      d_observedLineSegments(observedLineSegments),
      d_occlusionRays(occlusionRays),
      d_totalPixelCount(totalPixelCount),
      d_processedPixelCount(processedPixelCount),
      d_thinkCycleNumber(thinkCycleNumber)
    {}

    Maybe<Eigen::Vector2d> getBallObservation() const { return d_ballObservation; }
    bool isBallVisible() const { return d_ballObservation.hasValue(); }
    Vector2dVector getGoalObservations() const { return d_goalObservations; }
    Vector2dVector getTeamMateObservations() const { return d_teamMateObservations; }
    int getGoalObservationCount() const { return d_goalObservations.size(); }
    std::vector<OcclusionRay<ushort>> getOcclusionRays() const { return d_occlusionRays; }
    std::vector<LineSegment2i> getObservedLineSegments() const { return d_observedLineSegments; }
    long getTotalPixelCount() const { return d_totalPixelCount; }
    long getProcessedPixelCount() const { return d_processedPixelCount; }
    float getProcessedPixelRatio() const { return (float)d_processedPixelCount/d_totalPixelCount; }
    ulong getThinkCycleNumber() const { return d_thinkCycleNumber; }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    Maybe<Eigen::Vector2d> d_ballObservation;
    Vector2dVector d_goalObservations;
    Vector2dVector d_teamMateObservations;
    std::vector<LineSegment2i> d_observedLineSegments;
    std::vector<OcclusionRay<ushort>> d_occlusionRays;
    long d_totalPixelCount;
    long d_processedPixelCount;
    ulong d_thinkCycleNumber;
  };

  template<typename TBuffer>
  inline void CameraFrameState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("thinkCycle");
      writer.Uint64(d_thinkCycleNumber);

      writer.String("ball");
      if (d_ballObservation.hasValue())
      {
        writer.StartArray();
        writer.Double(d_ballObservation->x(), "%.1f");
        writer.Double(d_ballObservation->y(), "%.1f");
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
          writer.Double(goalPos.x(), "%.1f");
          writer.Double(goalPos.y(), "%.1f");
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
          writer.Double(teamMatePos.x(), "%.1f");
          writer.Double(teamMatePos.y(), "%.1f");
          writer.EndArray();
        }
      }
      writer.EndArray();

      writer.String("lines");
      writer.StartArray();
      {
        for (LineSegment2i const& lineSeg : d_observedLineSegments)
        {
          writer.StartArray();
          writer.Double(lineSeg.p1().x(), "%.1f");
          writer.Double(lineSeg.p1().y(), "%.1f");
          writer.Double(lineSeg.p2().x(), "%.1f");
          writer.Double(lineSeg.p2().y(), "%.1f");
          writer.EndArray();
        }
      }
      writer.EndArray();

      writer.String("occlusionRays");
      writer.StartArray();
      {
        for (auto const& ray : d_occlusionRays)
        {
          writer.StartArray();
          writer.Uint(ray.near().x());
          writer.Uint(ray.near().y());
          writer.Uint(ray.far().x());
          writer.Uint(ray.far().y());
          writer.EndArray();
        }
      }
      writer.EndArray();

      writer.String("totalPixelCount");
      writer.Uint64(d_totalPixelCount);

      writer.String("processedPixelCount");
      writer.Uint64(d_processedPixelCount);
    }
    writer.EndObject();
  }
}

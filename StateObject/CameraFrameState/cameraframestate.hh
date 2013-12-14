#pragma once

#include <Eigen/Core>

#include <vector>

#include "../stateobject.hh"
#include "../../util/Maybe.hh"
#include "../../geometry/LineSegment2i.hh"

namespace bold
{
  class CameraFrameState : public StateObject
  {
  public:
    CameraFrameState(Maybe<Eigen::Vector2d> ballObservation,
                     std::vector<Eigen::Vector2d> goalObservations,
                     std::vector<LineSegment2i> observedLineSegments,
                     long totalPixelCount, long processedPixelCount
                    )
    : d_ballObservation(ballObservation),
      d_goalObservations(goalObservations),
      d_observedLineSegments(observedLineSegments),
      d_totalPixelCount(totalPixelCount),
      d_processedPixelCount(processedPixelCount)
    {}

    Maybe<Eigen::Vector2d> getBallObservation() const { return d_ballObservation; }
    std::vector<Eigen::Vector2d> getGoalObservations() const { return d_goalObservations; }
    std::vector<LineSegment2i> getObservedLineSegments() const { return d_observedLineSegments; }
    long getTotalPixelCount() const { return d_totalPixelCount; }
    long getProcessedPixelCount() const { return d_processedPixelCount; }
    float getProcessedPixelRatio() const { return (float)d_processedPixelCount/d_totalPixelCount; }

    bool isBallVisible() const { return d_ballObservation.hasValue(); }

    virtual void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    Maybe<Eigen::Vector2d> d_ballObservation;
    std::vector<Eigen::Vector2d> d_goalObservations;
    std::vector<LineSegment2i> d_observedLineSegments;
    long d_totalPixelCount;
    long d_processedPixelCount;
  };
}

#pragma once

#include "../stateobject.hh"
#include "../../stats/average.hh"

#include <vector>
#include <Eigen/Geometry>

namespace bold
{
  /// Indicates which team owns a goal post.
  enum class GoalLabel
  {
    /// The ownership of the goal post could not be determined.
    Unknown,
    /// Our goal. Defend it!
    Ours,
    /// Their goal. Attack it!
    Theirs
  };

  std::string getGoalLabelName(GoalLabel label);

  //////////////////////////////////////////////////////////////////////////////

  class GoalEstimate
  {
  public:
    GoalEstimate(Average<Eigen::Vector3d> estimate, GoalLabel label)
    : d_estimate(estimate),
      d_label(label)
    {}

    Eigen::Vector3d getAverage() const { return d_estimate.getAverage(); }
    int getCount() const { return d_estimate.getCount(); }
    GoalLabel getLabel() const { return d_label; }

  private:
    Average<Eigen::Vector3d> d_estimate;
    GoalLabel d_label;
  };

  //////////////////////////////////////////////////////////////////////////////

  class StationaryMapState : public StateObject
  {
  public:
    StationaryMapState(std::vector<Average<Eigen::Vector3d>> ballEstimates,
                       std::vector<Average<Eigen::Vector3d>> goalEstimates,
                       std::vector<Average<Eigen::Vector3d>> teammateEstimates);

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

    std::vector<Average<Eigen::Vector3d>> const& getBallEstimates() const { return d_ballEstimates; };
    std::vector<Average<Eigen::Vector3d>> const& getTeammateEstimates() const { return d_teammateEstimates; };
    std::vector<GoalEstimate> const& getGoalEstimates() const { return d_goalEstimates; };

    uint countGoalsWithSamples(int sampleThreshold) const;
    bool hasBallWithSamples(int sampleThreshold) const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  private:
    static bool compareAverages(Average<Eigen::Vector3d> const& a, Average<Eigen::Vector3d> const& b)
    {
      return a.getCount() > b.getCount();
    }

    static std::vector<GoalEstimate> labelGoalObservations(
      std::vector<Average<Eigen::Vector3d>> const& teammateEstimates,
      std::vector<Average<Eigen::Vector3d>> const& goalEstimates);

    std::vector<Average<Eigen::Vector3d>> d_ballEstimates;
    std::vector<Average<Eigen::Vector3d>> d_teammateEstimates;
    std::vector<GoalEstimate> d_goalEstimates;
  };
}

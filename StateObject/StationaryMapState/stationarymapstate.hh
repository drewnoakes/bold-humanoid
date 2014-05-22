#pragma once

#include "../stateobject.hh"
#include "../../Kick/kick.hh"
#include "../../stats/average.hh"

#include <vector>
#include <Eigen/Geometry>

namespace bold
{
  /// Indicates which team owns a goal post.
  enum class GoalLabel
  {
    /// The ownership of the goal post could not be determined.
    Unknown = 0,
    /// Our goal. Defend it!
    Ours = 1,
    /// Their goal. Attack it!
    Theirs = 2
  };

  std::string getGoalLabelName(GoalLabel label);

  //////////////////////////////////////////////////////////////////////////////

  class GoalEstimate
  {
  public:
    GoalEstimate() = default;

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

  class KickResult
  {
  public:
    KickResult(std::shared_ptr<Kick const> kick, Eigen::Vector2d const& endPos, bool isOnTarget)
    : d_kick(kick),
      d_endPos(endPos),
      d_isOnTarget(isOnTarget)
    {};

    std::string getId() const { return d_kick->getId(); }
    std::shared_ptr<Kick const> getKick() const { return d_kick; }
    Eigen::Vector2d getEndPos() const { return d_endPos; }
    bool isOnTarget() const { return d_isOnTarget; }

  private:
    std::shared_ptr<Kick const> d_kick;
    Eigen::Vector2d d_endPos;
    bool d_isOnTarget;
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
    std::vector<Average<Eigen::Vector3d>> const& getTeammateEstimates() const { return d_keeperEstimates; };
    std::vector<GoalEstimate> const& getGoalEstimates() const { return d_goalEstimates; };

    bool hasEnoughBallObservations() const { return existsWithSamples(d_ballEstimates, BallSamplesNeeded); };
    bool hasEnoughGoalObservations() const { return countWithSamples(d_goalEstimates, GoalSamplesNeeded) >= 2; };
    bool hasEnoughBallAndGoalObservations() const { return hasEnoughBallObservations() && hasEnoughGoalObservations(); };

    bool canKick() const { return d_selectedKick != nullptr; }
    std::shared_ptr<Kick const> getSelectedKick() const { return d_selectedKick; }

    double getTurnAngleRads() const { return d_turnAngleRads; };

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

  private:
    static constexpr int GoalSamplesNeeded = 10; // TODO magic number!!
    static constexpr int BallSamplesNeeded = 20; // TODO magic number!!
    static constexpr int KeeperSamplesNeeded = 5; // TODO magic number!!

    static bool compareAverages(Average<Eigen::Vector3d> const& a, Average<Eigen::Vector3d> const& b)
    {
      return a.getCount() > b.getCount();
    }

    static std::vector<GoalEstimate> labelGoalObservations(
      std::vector<Average<Eigen::Vector3d>> const& keeperEstimates,
      std::vector<Average<Eigen::Vector3d>> const& goalEstimates);

    void calculateTurnAngle();
    void selectKick();

    template<typename T>
    inline static bool existsWithSamples(std::vector<T> const& estimates, int sampleThreshold);
    template<typename T>
    inline static uint countWithSamples(std::vector<T> const& estimates, int sampleThreshold);

    std::vector<Average<Eigen::Vector3d>> d_ballEstimates;
    std::vector<Average<Eigen::Vector3d>> d_keeperEstimates;
    std::vector<GoalEstimate> d_goalEstimates;
    std::vector<KickResult> d_possibleKicks;
    std::shared_ptr<Kick const> d_selectedKick;
    double d_turnAngleRads;
  };

  template<typename T>
  bool StationaryMapState::existsWithSamples(std::vector<T> const& estimates, int sampleThreshold)
  {
    return estimates.size() && estimates[0].getCount() >= sampleThreshold;
  }

  template<typename T>
  uint StationaryMapState::countWithSamples(std::vector<T> const& estimates, int sampleThreshold)
  {
    uint count = 0;
    for (auto const& estimate : estimates)
    {
      if (estimate.getCount() >= sampleThreshold)
        count++;
      else
        break; // list is ordered
    }
    return count;
  }
}

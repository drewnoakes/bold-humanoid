#pragma once

#include "../stateobject.hh"
#include "../../FieldMap/fieldmap.hh"
#include "../../Kick/kick.hh"
#include "../../Math/math.hh"
#include "../../OcclusionRay/occlusionray.hh"
#include "../../stats/average.hh"
#include "../../geometry/LineSegment/LineSegment2/linesegment2.hh"

#include <array>
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

  /** Models the estimated position and ownership of a pair of observed goal posts. */
  class GoalEstimate
  {
  public:
    GoalEstimate(Eigen::Vector2d const& post1Pos, Eigen::Vector2d const& post2Pos, GoalLabel label)
    : d_post1Pos(post1Pos),
      d_post2Pos(post2Pos),
      d_label(label)
    {}

    Eigen::Vector2d getPost1Pos() const { return d_post1Pos; }
    Eigen::Vector2d getPost2Pos() const { return d_post2Pos; }

    GoalLabel getLabel() const { return d_label; }

    Eigen::Vector2d getMidpoint(double ratio = 0.5) const { return Math::lerp(ratio, d_post1Pos, d_post2Pos); }

    bool isTowards(double endBallAngle) const;

    GoalEstimate estimateOppositeGoal(GoalLabel label) const;

    LineSegment2d lineSegment2d() const { return LineSegment2d(d_post1Pos, d_post2Pos); }

  private:
    Eigen::Vector2d d_post1Pos;
    Eigen::Vector2d d_post2Pos;
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

  // TODO rename as OpenFieldMap or similar
  class RadialOcclusionMap
  {
  public:
    void add(OcclusionRay<double> const& ray);

    void reset();

    double getOcclusionDistance(double angle) const;
    double getFieldDistance(double angle) const;

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    static constexpr int NumberOfBuckets = 144;

    static uint wedgeIndexForAngle(double angle);
    static double angleForWedgeIndex(uint index);

    std::array<Average<double>,NumberOfBuckets> d_nearWedges;
    std::array<Average<double>,NumberOfBuckets> d_farWedges;
  };

  template<typename TBuffer>
  inline void RadialOcclusionMap::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    auto writePoint = [&](uint index, const char* name, std::array<Average<double>,NumberOfBuckets> const& wedges)
    {
      writer.String(name);
      writer.StartObject();
      {
        writer.String("dist");
        writer.Double(wedges.at(index).getAverage(), "%.3f");
        writer.String("count");
        writer.Uint(wedges.at(index).getCount());
      }
      writer.EndObject();
    };

    writer.StartObject();
    {
      writer.String("divisions");
      writer.Uint(NumberOfBuckets);
      writer.String("slices");
      writer.StartArray();
      {
        for (uint index = 0; index < NumberOfBuckets; index++)
        {
          bool hasNear = d_nearWedges[index].getCount() != 0;
          bool hasFar = d_farWedges[index].getCount() != 0;

          if (!hasNear && !hasFar)
            continue;

          writer.StartObject();
          {
            writer.String("angle");
            writer.Double(angleForWedgeIndex(index), "%.3f");
            if (hasNear) writePoint(index, "near", d_nearWedges);
            if (hasFar)  writePoint(index, "far", d_farWedges);
          }
          writer.EndObject();
        }
      }
      writer.EndArray();
    }
    writer.EndObject();
  }

  //////////////////////////////////////////////////////////////////////////////

  class StationaryMapState : public StateObject
  {
  public:
    StationaryMapState(std::vector<Average<Eigen::Vector2d>> ballEstimates,
                       std::vector<Average<Eigen::Vector2d>> goalPostEstimates,
                       std::vector<Average<Eigen::Vector2d>> teammateEstimates,
                       RadialOcclusionMap occlusionMap);

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }

    std::vector<Average<Eigen::Vector2d>> const& getBallEstimates() const { return d_ballEstimates; }
    std::vector<Average<Eigen::Vector2d>> const& getTeammateEstimates() const { return d_keeperEstimates; }
    std::vector<Average<Eigen::Vector2d>> const& getGoalPostEstimates() const { return d_goalPostEstimates; }
    std::vector<GoalEstimate> const& getGoalEstimates() const { return d_goalEstimates; }

    bool hasEnoughBallObservations() const { return existsWithSamples(d_ballEstimates, BallSamplesNeeded); }
    bool hasEnoughGoalPostObservations() const { return d_goalEstimates.size() != 0; }
    bool hasEnoughBallAndGoalPostObservations() const { return hasEnoughBallObservations() && hasEnoughGoalPostObservations(); }

    /// Returns the number of goal post estimates that have enough observations
    long getSatisfactoryGoalPostCount() const;

    bool hasBallWithinDistance(double distance) const;

    bool needMoreSightingsOfGoalPostAt(Eigen::Vector2d goalPos) const;
    bool needMoreSightingsOfBallAt(Eigen::Vector2d ballPos) const;

    bool canKick() const { return d_selectedKick != nullptr; }
    std::shared_ptr<Kick const> getSelectedKick() const { return d_selectedKick; }

    double getTurnAngleRads() const { return d_turnAngleRads; };
    Eigen::Vector2d getTurnBallPos() const { return d_turnBallPos; };

    std::shared_ptr<Kick const> getTurnForKick() const { return d_turnForKick; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    static constexpr double BallMergeDistance = 0.3; // TODO magic number!!
    static constexpr double GoalPostMergeDistance = 0.5; // TODO magic number!!
    static constexpr double TeammateMergeDistance = 0.5; // TODO magic number!!
    static constexpr int GoalSamplesNeeded = 10; // TODO magic number!!
    static constexpr int BallSamplesNeeded = 20; // TODO magic number!!
    static constexpr int KeeperSamplesNeeded = 5; // TODO magic number!!

    /// Extracts goal pairs from individual post observations.
    static std::vector<std::pair<Average<Eigen::Vector2d>,Average<Eigen::Vector2d>>> pairGoalPosts(std::vector<Average<Eigen::Vector2d>> goalPostEstimates);

    static GoalLabel labelGoalByKeeperBallDistance(
      Average<Eigen::Vector2d> const& post1Pos,
      Average<Eigen::Vector2d> const& post2Pos,
      FieldSide ballSideEstimate);

    static GoalLabel labelGoalByKeeperBallPosition(
      Average<Eigen::Vector2d> const& post1Pos,
      Average<Eigen::Vector2d> const& post2Pos,
      Eigen::Vector2d const& keeperBallPos,
      Eigen::Vector2d const& agentBallPos);

    static GoalLabel labelGoalByKeeperObservations(
      Average<Eigen::Vector2d> const& post1Pos,
      Average<Eigen::Vector2d> const& post2Pos,
      std::vector<Average<Eigen::Vector2d>> keeperEstimates);

    /** Estimate the position of a point in the agent frame if the specified posts have the specified label. */
    static Eigen::Vector2d estimateWorldPositionForPoint(
      Eigen::Vector2d const& post1,
      Eigen::Vector2d const& post2,
      Eigen::Vector2d const& pointAgent,
      GoalLabel label);

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    template<typename T>
    static bool compareAverages(Average<T> const& a, Average<T> const& b)
    {
      return a.getCount() > b.getCount();
    }

    /// Attempts to label a pair of goal posts as being either ours, theirs or unknown.
    /// Uses techniques available in other labelGoalBy* functions.
    GoalLabel labelGoal(
      Average<Eigen::Vector2d> const& post1Pos,
      Average<Eigen::Vector2d> const& post2Pos,
      Maybe<Eigen::Vector2d> const& agentBallPos);

    /// Selects a kick (if there is one) which may be made immediately with a suitably positive outcome.
    /// If successful, canKick() will return true and getSelectedKick() returns the kick.
    void selectImmediateKick();

    /// Selects a turn angle, turn direction and kick (if there is one) which produce a suitably positive outcome.
    /// If successful, getTurnAngleRads() will return non-zero, getTurnBallPos() gives the position the ball
    /// should be kept in, and getTurnForKick() returns the kick.
    void calculateTurnAndKick();

    template<typename T>
    inline static bool existsWithSamples(std::vector<T> const& estimates, int sampleThreshold);
    template<typename T>
    inline static uint countWithSamples(std::vector<T> const& estimates, int sampleThreshold);

    std::vector<Average<Eigen::Vector2d>> d_ballEstimates;
    std::vector<Average<Eigen::Vector2d>> d_goalPostEstimates;
    std::vector<Average<Eigen::Vector2d>> d_keeperEstimates;
    RadialOcclusionMap d_occlusionMap;

    std::vector<GoalEstimate> d_goalEstimates;
    std::vector<KickResult> d_possibleKicks;
    std::shared_ptr<Kick const> d_selectedKick;
    double d_turnAngleRads;
    Eigen::Vector2d d_turnBallPos;
    std::shared_ptr<Kick const> d_turnForKick;
  };

  template<typename TBuffer>
  inline void StationaryMapState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("balls");
      writer.StartArray();
      for (auto const& estimate : d_ballEstimates)
      {
        writer.StartObject();
        {
          writer.String("pos");
          writer.StartArray();
          writer.Double(estimate.getAverage().x(), "%.3f");
          writer.Double(estimate.getAverage().y(), "%.3f");
          writer.EndArray();
          writer.String("count");
          writer.Int(estimate.getCount());
        }
        writer.EndObject();
      }
      writer.EndArray();

      writer.String("goalPosts");
      writer.StartArray();
      for (auto const& estimate : d_goalPostEstimates)
      {
        writer.StartObject();
        {
          writer.String("pos");
          writer.StartArray();
          writer.Double(estimate.getAverage().x(), "%.3f");
          writer.Double(estimate.getAverage().y(), "%.3f");
          writer.EndArray();
          writer.String("count");
          writer.Uint(estimate.getCount());
        }
        writer.EndObject();
      }
      writer.EndArray();

      writer.String("goals");
      writer.StartArray();
      for (auto const& estimate : d_goalEstimates)
      {
        writer.StartObject();
        {
          writer.String("post1");
          writer.StartArray();
          writer.Double(estimate.getPost1Pos().x(), "%.3f");
          writer.Double(estimate.getPost1Pos().y(), "%.3f");
          writer.EndArray();
          writer.String("post2");
          writer.StartArray();
          writer.Double(estimate.getPost2Pos().x(), "%.3f");
          writer.Double(estimate.getPost2Pos().y(), "%.3f");
          writer.EndArray();
          writer.String("label");
          writer.Uint(static_cast<uint>(estimate.getLabel()));
        }
        writer.EndObject();
      }
      writer.EndArray();

      writer.String("keepers");
      writer.StartArray();
      for (auto const& estimate : d_keeperEstimates)
      {
        writer.StartObject();
        {
          writer.String("pos");
          writer.StartArray();
          writer.Double(estimate.getAverage().x(), "%.3f");
          writer.Double(estimate.getAverage().y(), "%.3f");
          writer.EndArray();
          writer.String("count");
          writer.Int(estimate.getCount());
        }
        writer.EndObject();
      }
      writer.EndArray();

      writer.String("kicks");
      writer.StartArray();
      {
        for (auto const& kick : d_possibleKicks)
        {
          writer.StartObject();
          {
            writer.String("id");
            writer.String(kick.getId().c_str());
            Eigen::Vector2d const& endPos = kick.getEndPos();
            writer.String("endPos");
            writer.StartArray();
            writer.Double(endPos.x(), "%.3f");
            writer.Double(endPos.y(), "%.3f");
            writer.EndArray();
            writer.String("onTarget");
            writer.Bool(kick.isOnTarget());
            writer.String("selected");
            writer.Bool(kick.getKick() == d_selectedKick);
          }
          writer.EndObject();
        }
      }
      writer.EndArray();

      writer.String("openField");
      d_occlusionMap.writeJson(writer);

      writer.String("turnAngle");
      writer.Double(d_turnAngleRads, "%.3f");
      writer.String("turnBallPos");
      writer.StartArray();
      writer.Double(d_turnBallPos.x(), "%.3f");
      writer.Double(d_turnBallPos.y(), "%.3f");
      writer.EndArray();
    }
    writer.EndObject();
  }

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

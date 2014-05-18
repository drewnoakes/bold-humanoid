#pragma once

#include "../../geometry/Bounds.hh"
#include "../../util/Maybe.hh"
#include "../option.hh"

#include <Eigen/Core>

namespace bold
{
  class Agent;
  class MotionScriptModule;
  class MotionScriptOption;

  class Kick
  {
  public:
    Kick(std::string id, std::string scriptPath, Bounds2d ballBounds, Eigen::Vector2d endPos);

    /**
     * Given a ball position, returns whether the ball can be kicked and
     * where it is likely to end up.
     *
     * All positions are in the agent frame.
     */
    Maybe<Eigen::Vector2d> estimateEndPos(Eigen::Vector2d const& ballPos) const;

    std::string getId() const { return d_id; }

    std::shared_ptr<MotionScriptOption> createScriptOption(std::shared_ptr<MotionScriptModule> motionScriptModule) const;

  private:
    std::string d_id;
    std::string d_scriptPath;
    Bounds2d d_ballBounds;
    Eigen::Vector2d d_endPos;
  };

  //////////////////////////////////////////////////////////////////////////////

  class KickOption : public Option
  {
  public:
    static constexpr int GoalSamplesNeeded = 10; // TODO magic number!!
    static constexpr int BallSamplesNeeded = 20; // TODO magic number!!
    static constexpr int KeeperSamplesNeeded = 5; // TODO magic number!!

    KickOption(std::string const& id, Agent* agent);

    /** Indicates whether, given the current static map, the ball may be kicked in an advantageous way. */
    bool canKick() const;

    virtual std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    virtual void reset() override;

    virtual double hasTerminated() override;

    /// Indicates whether the StationaryMap contains enough data to attempt kick selection.
    bool canSelectKick() const;

  private:
    /** Selects the most advantageous kick from the current position, if any. */
    Kick const* selectKick() const;

    Agent* d_agent;
    bool d_hasRun;
    std::shared_ptr<MotionScriptOption> d_activeScript;
    std::vector<Kick> d_kicks;
  };
}

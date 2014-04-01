#pragma once

#include <vector>

#include "../stateobject.hh"
#include "../../AgentPosition/agentposition.hh"
#include "../../Clock/clock.hh"
#include "../../util/Maybe.hh"

namespace bold
{
  enum class PlayerRole
  {
    /// Robot is running, but paused or otherwise indisposed and should not be
    /// considered as actively on the team at present.
    Idle = 0,

    /// Robot is acting as the keeper, determined by uniform number.
    /// By convention, the keeper has unum 1.
    Keeper = 1,

    /// Robot is positioning to receive a pass towards the goal.
    Supporter = 2,

    /// Robot is claiming possession of the ball and advancing it towards the
    /// opponent's goal.
    Striker = 3,

    /// Robot is positioning so as to block an opponent's advance towards
    /// our goal.
    Defender = 4,

    /// Robot is acting as a keeper during a penalty shootout.
    PenaltyKeeper = 5,

    /// Robot is acting as a striker during a penalty shootout.
    PenaltyStriker = 6,

    /// Robot role is unknown.
    /// This status will not be transmitted by the Bold Hearts as our agents
    /// default to striker/keeper in the absence of sufficient information to
    /// decide otherwise.
    Other = 7
  };

  enum class PlayerActivity
  {
    /// Robot is moving to a supporting or defending position.
    Positioning = 0,

    /// Robot is moving towards the ball, either as a striker or a defender.
    ApproachingBall = 1,

    /// Robot has possession of the ball and is attacking the opponent's goal,
    /// as a striker.
    AttackingGoal = 2,

    /// Robot is not taking any action, as keeper, supporter, or defender.
    Waiting = 3,

    /// Robot activity is unknown.
    /// This status will not be transmitted by the Bold Hearts as the other
    /// enum members sufficiently cover our activities. This value may be
    /// seen when playing in a mixed team, however.
    Other = 4
  };

  enum class PlayerStatus
  {
    /// Robot is not doing anything, or is incapable. It may have fallen, or
    /// the game may be in a play mode that does not permit motion (eg. Set.)
    /// The activity should be set to Waiting.
    Inactive = 0,

    /// Robot is active and able.
    Active = 1,

    /// The robot has been penalised and is not permitted to take any action.
    Penalised = 2
  };

  class PlayerState
  {
  public:
    int uniformNumber;
    int teamNumber;

    PlayerStatus status;
    PlayerRole role;
    PlayerActivity activity;

    AgentPosition pos;
    double posConfidence;
    Maybe<Eigen::Vector2d> ballRelative;
//    double ballConfidence;

    /// Time that the message was received according to the agent's own clock.
    Clock::Timestamp updateTime;

    bool isMe() const;
  };

  class TeamState : public StateObject
  {
  public:
    TeamState(std::vector<PlayerState> playerStates)
    : d_playerStates(playerStates)
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

    bool empty() const { return d_playerStates.empty(); }

    std::vector<PlayerState> const& players() const { return d_playerStates; }

  private:
    std::vector<PlayerState> d_playerStates;
  };
}

#pragma once

#include <vector>

#include "../stateobject.hh"
#include "../../AgentPosition/agentposition.hh"
#include "../../Clock/clock.hh"
#include "../../FieldMap/fieldmap.hh"
#include "../../JsonWriter/jsonwriter.hh"
#include "../../util/Maybe.hh"

namespace bold
{
  typedef unsigned char uchar;

  constexpr uchar GOALIE_UNUM = 1;

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
    Other = 7,

    /// The robot will stand and kick the ball around in order to learn the
    /// outcome of specific kicks given starting ball positions.
    /// This is not a role in a normal game, and using roles to do this may not be the best way.
    KickLearner = 8,

    /// The robot will approach the ball and turn a random angle around it.
    BallCircler = 9,

    /// Robot stands straight, indicating whenever a whistle is heard.
    WhistleListener = 10
  };

  std::string getPlayerRoleString(PlayerRole role);

  std::ostream& operator<<(std::ostream &stream, PlayerRole const& role);

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

  std::string getPlayerActivityString(PlayerActivity activity);

  enum class PlayerStatus
  {
    /// Robot is not doing anything, or is incapable. It may have fallen, or
    /// the game may be in a play mode that does not permit motion (eg. Set.)
    /// The activity should be set to Waiting.
    Inactive = 0,

    /// Robot is active and able.
    Active = 1,

    /// The robot has been penalised and is not permitted to take any action.
    Penalised = 2,

    /// The robot is manually paused.
    Paused = 3
  };

  std::string getPlayerStatusString(PlayerStatus status);

  class PlayerState
  {
  public:
    uchar uniformNumber;
    uchar teamNumber;

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
    double getAgeMillis() const;
  };

  class TeamState : public StateObject
  {
  public:
    TeamState(std::vector<PlayerState> playerStates)
    : d_playerStates(playerStates)
    {}

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

    bool empty() const { return d_playerStates.empty(); }

    std::vector<PlayerState> const& players() const { return d_playerStates; }

    PlayerState const* getKeeperState() const
    {
      auto it = std::find_if(d_playerStates.begin(), d_playerStates.end(),
                             [](PlayerState const& p) { return p.role == PlayerRole::Keeper; });

      return it == d_playerStates.end() ? nullptr : &(*it);
    }

    std::vector<PlayerState> getBallObservers() const;

    bool isTeamMateInActivity(PlayerActivity activity) const;

    FieldSide getKeeperBallSideEstimate() const;

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    std::vector<PlayerState> d_playerStates;
  };

  template<typename TBuffer>
  inline void TeamState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
    writer.StartObject();
    {
      writer.String("players");
      writer.StartArray();
      {
        for (PlayerState const& player : d_playerStates)
        {
          writer.StartObject();
          {
            writer.String("unum");
            writer.Uint(player.uniformNumber);
            writer.String("team");
            writer.Uint(player.teamNumber);
            writer.String("isMe");
            writer.Bool(player.isMe());
            writer.String("activity");
            writer.Int(static_cast<int>(player.activity));
            writer.String("status");
            writer.Int(static_cast<int>(player.status));
            writer.String("role");
            writer.Int(static_cast<int>(player.role));
            writer.String("pos");
            writer.StartArray();
            JsonWriter::swapNaN(writer, player.pos.x(), "%.3f");
            JsonWriter::swapNaN(writer, player.pos.y(), "%.3f");
            JsonWriter::swapNaN(writer, player.pos.theta(), "%.4f");
            writer.EndArray();
            writer.String("posConfidence");
            writer.Double(player.posConfidence, "%.3f");
            writer.String("ballRelative");
            writer.StartArray();
            {
              if (player.ballRelative.hasValue())
              {
                writer.Double(player.ballRelative->x(), "%.3f");
                writer.Double(player.ballRelative->y(), "%.3f");
              }
            }
            writer.EndArray();
            writer.String("updateTime");
            writer.Uint64(Clock::timestampToMillis(player.updateTime));
          }
          writer.EndObject();
        }
      }
      writer.EndArray();
    }
    writer.EndObject();
  }
}

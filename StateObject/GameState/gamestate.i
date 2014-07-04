%{
#include <StateObject/GameState/gamestate.hh>
%}

namespace bold
{
  #define MAX_NUM_PLAYERS 11

  typedef unsigned char  uint8;
  typedef unsigned short uint16;
  typedef unsigned int   uint32;

  class PlayerInfo
  {
  public:
    bool hasPenalty() const;
    uint16 getPenaltyType() const;
    std::string getPenaltyTypeString();

    /** estimate of time till unpenalised */
    uint16 getSecondsUntilPenaltyLifted() const;
  };

  class TeamInfo
  {
  public:
    uint8 getTeamNumber() const;
    uint8 isBlueTeam() const;
    uint8 getGoalColour() const;
    uint8 getScore() const;
    PlayerInfo const& getPlayer(uint8 unum) const;
  };

  class GameState : public StateObject
  {
  public:
    int getSecondsRemaining() const;
    uint8 getPlayMode() const;
    std::string getPlayModeString() const;

    uint32 getVersion() const;
    uint8 getPlayersPerTeam() const;
    bool isFirstHalf() const;
    /** Index of the next team to kick off. Zero or one. */
    uint8 getNextKickOffTeamIndex() const;
    bool isPenaltyShootout() const;
    bool isOvertime() const;
    uint8 getSecondsSinceLastDropIn() const;
    uint8 getLastDropInTeamNumber() const;
    TeamInfo const& teamInfo1() const;
    TeamInfo const& teamInfo2() const;

    TeamInfo const& teamInfo(unsigned teamNumber) const;

    PlayerInfo const& playerInfo(unsigned team, unsigned unum) const;
  };
}

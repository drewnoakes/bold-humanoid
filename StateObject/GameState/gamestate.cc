#include "gamestate.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

GameResult GameState::getGameResult() const
{
  if (isFirstHalf() || getPlayMode() != PlayMode::FINISHED)
    return GameResult::Undecided;

  uint8_t ourScore = getMyTeam().getScore();
  uint8_t theirScore = getOpponentTeam().getScore();

  if (ourScore > theirScore)
    return GameResult::Victory;

  if (ourScore < theirScore)
    return GameResult::Loss;

  // TODO had to know if there's going to be extra time or a penalty shoot out in case of draw

  return GameResult::Undecided;
}

bool GameState::isWithinTenSecondsOfKickOff(Team team) const
{
  static uchar teamNumber = static_cast<uchar>(Config::getStaticValue<int>("team-number"));
  uchar nextKickOffTeamIndex = getNextKickOffTeamIndex();

  bool isOurKickOff = nextKickOffTeamIndex == getTeamIndex(teamNumber);
  bool isOurTeam = team == Team::Us;

  PlayMode playMode = getPlayMode();
  int secondaryTime = getSecondaryTime();

  return isOurTeam == isOurKickOff && playMode == PlayMode::PLAYING && secondaryTime > 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string bold::getPlayModeName(PlayMode playMode)
{
  switch (playMode)
  {
    case PlayMode::INITIAL:  return "Initial";
    case PlayMode::READY:    return "Ready";
    case PlayMode::SET:      return "Set";
    case PlayMode::PLAYING:  return "Playing";
    case PlayMode::FINISHED: return "Finished";
    default:
      std::stringstream msg;
      msg << "Unsupported PlayMode enum value: " << (int)playMode;
      throw std::runtime_error(msg.str());
  }
}

std::string bold::getPeriodTypeName(PeriodType periodType)
{
  switch (periodType)
  {
    case PeriodType::NORMAL:           return "Normal";
    case PeriodType::PENALTY_SHOOTOUT: return "Penalty shootout";
    case PeriodType::OVERTIME:         return "Overtime";
    case PeriodType::TIMEOUT:          return "Timeout";
    default:
      std::stringstream msg;
      msg << "Unsupported PeriodType enum value: " << (int)periodType;
      throw std::runtime_error(msg.str());
  }
}

std::string bold::getPenaltyTypeName(PenaltyType penaltyType)
{
  switch (penaltyType)
  {
    case PenaltyType::NONE:                return "No Penalty";
    case PenaltyType::BALL_MANIPULATION:   return "Ball Manipulation";
    case PenaltyType::PHYSICAL_CONTACT:    return "Physical Contact";
    case PenaltyType::ILLEGAL_ATTACK:      return "Illegal Attack";
    case PenaltyType::ILLEGAL_DEFENSE:     return "Illegal Defense";
    case PenaltyType::PICKUP_OR_INCAPABLE: return "Pickup or Incapable";
    case PenaltyType::SERVICE:             return "Service";
    case PenaltyType::SUBSTITUTE:          return "Substitute";
    case PenaltyType::MANUAL:              return "Manual";
    default:
      std::stringstream msg;
      msg << "Unsupported PenaltyType enum value: " << (int)penaltyType;
      throw std::runtime_error(msg.str());
  }
}

std::string bold::getLeagueName(League league)
{
  switch (league)
  {
    case League::SPL:               return "SPL";
    case League::SPLDropIn:         return "SPL Drop In";
    case League::HumanoidKidSize:   return "Humanoid Kid Size";
    case League::HumanoidTeenSize:  return "Humanoid Teen Size";
    case League::HumanoidAdultSize: return "Humanoid Adult Size";
    default:
      std::stringstream msg;
      msg << "Unsupported League enum value: " << (int)league;
      throw std::runtime_error(msg.str());
  }
}

std::string bold::getTeamColorName(TeamColor teamColor)
{
  switch (teamColor)
  {
    case TeamColor::Red:  return "Red";
    case TeamColor::Blue: return "Blue";
    default:
      std::stringstream msg;
      msg << "Unsupported TeamColor enum value: " << (int)teamColor;
      throw std::runtime_error(msg.str());
  }
}

std::string bold::getGameResultName(GameResult gameResult)
{
  switch (gameResult)
  {
    case GameResult::Undecided: return "Undecided";
    case GameResult::Victory:   return "Victory";
    case GameResult::Loss:      return "Loss";
    case GameResult::Draw:      return "Draw";
    default:
      std::stringstream msg;
      msg << "Unsupported GameResult enum value: " << (int)gameResult;
      throw std::runtime_error(msg.str());
  }
}

std::string bold::getRobotStatusMessageTypeName(RobotStatusMessageType status)
{
  switch (status)
  {
    case RobotStatusMessageType::MANUALLY_PENALISED:   return "Manually Penalised";
    case RobotStatusMessageType::MANUALLY_UNPENALISED: return "Manually Unpenalised";
    case RobotStatusMessageType::ALIVE:                return "Alive";
    default:
      std::stringstream msg;
      msg << "Unsupported RobotStatusMessageType enum value: " << (int)status;
      throw std::runtime_error(msg.str());
  }
}

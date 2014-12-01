#include "openteamcommunicator.hh"

#include "../../Agent/agent.hh"
#include "../../BehaviourControl/behaviourcontrol.hh"
#include "../../Clock/clock.hh"
#include "../../Config/config.hh"
#include "../../Math/math.hh"
#include "../../RoleDecider/roledecider.hh"
#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../../StateObject/TeamState/teamstate.hh"
#include "../../StateObject/WorldFrameState/worldframestate.hh"

#include "../../mitecom/mitecom-network.h"
#include "../../mitecom/mitecom-handler.h"

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <memory>

#include <Eigen/Core>

using namespace std;
using namespace bold;
using namespace Eigen;

OpenTeamCommunicator::OpenTeamCommunicator(shared_ptr<BehaviourControl> behaviourControl)
: StateObserver::StateObserver("Open Team Communicator", ThreadId::ThinkLoop),
  d_behaviourControl(behaviourControl),
  d_teamNumber((uchar)Config::getStaticValue<int>("team-number")),
  d_uniformNumber((uchar)Config::getStaticValue<int>("uniform-number")),
  d_localPort(Config::getStaticValue<int>("mitecom.local-port")),
  d_remotePort(Config::getStaticValue<int>("mitecom.remote-port")),
  d_sendPeriodSeconds(Config::getSetting<double>("mitecom.send-period-seconds")),
  d_maxPlayerDataAgeMillis(Config::getSetting<int>("mitecom.max-player-data-age-millis")),
  d_lastBroadcast(0)
{
  d_types.push_back(typeid(AgentFrameState));
  d_types.push_back(typeid(WorldFrameState));

  // Open listening UDP socket on given port
  d_sock = mitecom_open(d_localPort);

  if (d_sock == -1)
    log::error("OpenTeamCommunicator::observeTyped") << "Failure binding socket to port " << d_localPort;
}

void OpenTeamCommunicator::observe(SequentialTimer& timer)
{
  auto now = Clock::getTimestamp();

  if (Clock::getSecondsSince(d_lastBroadcast) > d_sendPeriodSeconds->getValue())
  {
    // Create a PlayerState object for this agent
    PlayerState playerState;
    playerState.uniformNumber = d_uniformNumber;
    playerState.teamNumber = d_teamNumber;

    playerState.activity = d_behaviourControl->getPlayerActivity();
    playerState.status = d_behaviourControl->getPlayerStatus();
    playerState.role = d_behaviourControl->getPlayerRole();

    if (playerState.status != PlayerStatus::Paused && playerState.status != PlayerStatus::Penalised)
    {
      auto const& agentFrameState = State::get<AgentFrameState>();
      if (agentFrameState)
      {
        static Maybe<Eigen::Vector2d> stickyBallPos;
        static Clock::Timestamp lastBallTime = 0;

        auto const& ballObservation = agentFrameState->getBallObservation();
        if (ballObservation.hasValue())
        {
          // We see the ball, so set it
          playerState.ballRelative = (Vector2d)ballObservation->head<2>();
          // Remember it for later
          stickyBallPos = (Vector2d)ballObservation->head<2>();
          lastBallTime = Clock::getTimestamp();
        }
        else
        {
          // No ball this cycle, but if within a short enough period, use the prior value
          // TODO set playerState.ballConfidence
          if (Clock::getSecondsSince(lastBallTime) < 5.0) // TODO magic number!!!
            playerState.ballRelative = stickyBallPos;
          else
            stickyBallPos = Maybe<Vector2d>::empty();
        }
      }

      auto const& worldFrameState = State::get<WorldFrameState>();
      if (worldFrameState)
      {
        playerState.pos = worldFrameState->getPosition();
        // TODO come up with a proper confidence value
        playerState.posConfidence = 1.0;
      }
      else
      {
        playerState.posConfidence = 0.0;
      }
    }
    else
    {
      playerState.posConfidence = 0.0;
    }

    sendData(playerState);

    d_lastBroadcast = now;
  }
}

void OpenTeamCommunicator::receiveData()
{
  // TODO this buffer size is probably way too large
  const uint BUFFER_SIZE = 32767;

  char buffer[BUFFER_SIZE];

  bool updated = false;

  while (true)
  {
    // Receive a message (if available), this call is nonblocking
    ssize_t messageLength = mitecom_receive(d_sock, buffer, BUFFER_SIZE);

    if (messageLength <= 0)
      break;

    // Message received, update our view of the Team
    MixedTeamMate message = MixedTeamParser::parseIncoming(buffer, messageLength, d_teamNumber);

    // Ignore messages from ourselves.
    if (message.robotID == d_uniformNumber)
      continue;

    // An ID of -1 (0xFFFF) indicates that the message was invalid.
    if (message.robotID == 0xFFFF)
    {
      static bool errorYet = false;
      if (!errorYet)
      {
        log::error("OpenTeamCommunicator::receiveData") << "Error parsing open mitecom message (will not report future errors)";
        errorYet = true;
      }
      continue;
    }

    log::verbose("OpenTeamCommunicator::receiveData") << "Received mitecom message with length " << messageLength << " bytes from robot " << message.robotID;

    PlayerState teammateState;
    teammateState.uniformNumber = message.robotID;
    // TODO need a way to verify that this message is actually from *our* team
    teammateState.teamNumber = d_teamNumber;

    auto posX = message.data.find(ROBOT_ABSOLUTE_X);
    auto posY = message.data.find(ROBOT_ABSOLUTE_Y);
    auto theta = message.data.find(ROBOT_ABSOLUTE_ORIENTATION);
    if (posX != message.data.end() && posX != message.data.end() && posX != message.data.end())
      teammateState.pos = AgentPosition(posX->second/1000.0, posY->second/1000.0, Math::degToRad(theta->second));

    auto posConfidence = message.data.find(ROBOT_ABSOLUTE_BELIEF);
    if (posConfidence != message.data.end())
      teammateState.posConfidence = posConfidence->second/255.0;

    auto ballX = message.data.find(BALL_RELATIVE_X);
    auto ballY = message.data.find(BALL_RELATIVE_Y);
    if (ballX != message.data.end() && ballY != message.data.end())
      teammateState.ballRelative = Vector2d(ballX->second/1000.0, ballY->second/1000.0);

    auto action = message.data.find(ROBOT_CURRENT_ACTION);
    if (action != message.data.end())
      teammateState.activity = decodePlayerActivity(action->second);

    auto role = message.data.find(ROBOT_CURRENT_ROLE);
    if (role != message.data.end())
      teammateState.role = decodePlayerRole(role->second);

    auto state = message.data.find(ROBOT_CURRENT_STATE);
    if (state != message.data.end())
      teammateState.status = decodePlayerStatus(state->second);

    mergePlayerState(teammateState);

    updated = true;
  }

  if (updated)
    updateStateObject();
}

void OpenTeamCommunicator::sendData(PlayerState& state)
{
  mergePlayerState(state);

  // NOTE protocol uses millimeters, we use meters -- scale values

  MixedTeamMate myInformation;
  myInformation.robotID = d_uniformNumber;
  myInformation.data[ROBOT_CURRENT_ACTION] = encodePlayerActivity(state.activity);
  myInformation.data[ROBOT_CURRENT_ROLE]   = encodePlayerRole(state.role);
  myInformation.data[ROBOT_CURRENT_STATE]  = encodePlayerStatus(state.status);
  myInformation.data[ROBOT_ABSOLUTE_X] = static_cast<int>(state.pos.x() * 1000);
  myInformation.data[ROBOT_ABSOLUTE_Y] = static_cast<int>(state.pos.y() * 1000);
  myInformation.data[ROBOT_ABSOLUTE_ORIENTATION] = Math::radToDeg(state.pos.theta());
  myInformation.data[ROBOT_ABSOLUTE_BELIEF] = Math::clamp(static_cast<int>(state.posConfidence * 255), 0, 255);

  if (state.ballRelative.hasValue())
  {
    myInformation.data[BALL_RELATIVE_X] = state.ballRelative->x() * 1000;
    myInformation.data[BALL_RELATIVE_Y] = state.ballRelative->y() * 1000;
  }

  uint32_t messageDataLength = 0;

  // Serialize and broadcast data
  auto messageData = unique_ptr<MixedTeamCommMessage>(MixedTeamParser::create(&messageDataLength, myInformation, d_teamNumber, d_uniformNumber));
  ASSERT(messageData != nullptr);
  ASSERT(messageDataLength > 0);

  mitecom_broadcast(d_sock, d_remotePort, messageData.get(), messageDataLength);

  updateStateObject();
}

void OpenTeamCommunicator::updateStateObject()
{
  // Remove players where data has not been heard for some time
  d_players.erase(std::remove_if(d_players.begin(), d_players.end(),
    [](PlayerState const& player) { return player.getAgeMillis() > d_maxPlayerDataAgeMillis->getValue(); }));

  State::make<TeamState>(d_players);
}

void OpenTeamCommunicator::mergePlayerState(PlayerState& state)
{
  // Remember the last time (i.e. now) that we heard from this robot
  state.updateTime = Clock::getTimestamp();

  auto it = find_if(d_players.begin(), d_players.end(), [&state](PlayerState const& s) { return state.uniformNumber == s.uniformNumber; });

  if (it == d_players.end())
  {
    if (state.uniformNumber != d_uniformNumber || state.teamNumber != d_teamNumber)
      log::info("OpenTeamCommunicator::receiveData") << "First message seen from player " << (int)state.uniformNumber;

    d_players.push_back(state);
  }
  else
  {
    *it = state;
  }
}

//
//// Functions for conversion between our enums, and the mitecom ones
//

PlayerRole OpenTeamCommunicator::decodePlayerRole(int value)
{
  switch (value)
  {
    case ROLE_DEFENDER:  return PlayerRole::Defender;
    case ROLE_GOALIE:    return PlayerRole::Keeper;
    case ROLE_IDLING:    return PlayerRole::Idle;
    case ROLE_STRIKER:   return PlayerRole::Striker;
    case ROLE_SUPPORTER: return PlayerRole::Supporter;

    case ROLE_OTHER:
    default:             return PlayerRole::Other;
  }
}

PlayerActivity OpenTeamCommunicator::decodePlayerActivity(int value)
{
  switch (value)
  {
    case ACTION_GOING_TO_BALL:   return PlayerActivity::ApproachingBall;
    case ACTION_POSITIONING:     return PlayerActivity::Positioning;
    case ACTION_TRYING_TO_SCORE: return PlayerActivity::AttackingGoal;
    case ACTION_WAITING:         return PlayerActivity::Waiting;

    case ACTION_UNDEFINED:
    default:                     return PlayerActivity::Other;
  }
}

PlayerStatus OpenTeamCommunicator::decodePlayerStatus(int value)
{
  switch (value)
  {
    case STATE_ACTIVE:    return PlayerStatus::Active;
    case STATE_PENALIZED: return PlayerStatus::Penalised;

    case STATE_INACTIVE:
    default:              return PlayerStatus::Inactive;
  }
}


int OpenTeamCommunicator::encodePlayerRole(PlayerRole role)
{
  switch (role)
  {
    case PlayerRole::Defender:  return ROLE_DEFENDER;
    case PlayerRole::Keeper:    return ROLE_GOALIE;
    case PlayerRole::Idle:      return ROLE_IDLING;
    case PlayerRole::Striker:   return ROLE_STRIKER;
    case PlayerRole::Supporter: return ROLE_SUPPORTER;

    case PlayerRole::Other:
    default:                    return ROLE_OTHER;
  }
}

int OpenTeamCommunicator::encodePlayerActivity(PlayerActivity activity)
{
  switch (activity)
  {
    case PlayerActivity::ApproachingBall: return ACTION_GOING_TO_BALL;
    case PlayerActivity::Positioning:     return ACTION_POSITIONING;
    case PlayerActivity::AttackingGoal:   return ACTION_TRYING_TO_SCORE;
    case PlayerActivity::Waiting:         return ACTION_WAITING;

    case PlayerActivity::Other:
    default:                              return ACTION_UNDEFINED;
  }
}

int OpenTeamCommunicator::encodePlayerStatus(PlayerStatus status)
{
  switch (status)
  {
    case PlayerStatus::Active:    return STATE_ACTIVE;
    case PlayerStatus::Penalised: return STATE_PENALIZED;
    case PlayerStatus::Inactive:
    default:                      return STATE_INACTIVE;
  }
}

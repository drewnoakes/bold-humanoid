#include "openteamcommunicator.hh"

#include "../../Clock/clock.hh"
#include "../../Math/math.hh"
#include "../../State/state.hh"
#include "../../StateObject/OpenTeamState/openteamstate.hh"

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

using namespace std;
using namespace bold;
using namespace Eigen;

// TODO Move hard-coded values into config file
#define BUFFER_SIZE SHRT_MAX
#define LOCAL_PORT 8081
#define REMOTE_PORT 8082
#define ROLE_IN_TEAM ROLE_OTHER

OpenTeamCommunicator::OpenTeamCommunicator(unsigned teamNumber, unsigned uniformNumber)
: StateObserver::StateObserver("Open Team Communicator", ThreadId::ThinkLoop),
  d_teamNumber(teamNumber),
  d_uniformNumber(uniformNumber),
  d_lastBroadcast(Clock::getTimestamp())
{
  d_types.push_back(typeid(AgentFrameState));
  d_types.push_back(typeid(WorldFrameState));

  // Open listening UDP socket on given port
  d_sock = mitecom_open(LOCAL_PORT);

  if (d_sock == -1)
    log::error("OpenTeamCommunicator::observeTyped") << "Failure binding socket to port " << LOCAL_PORT;
}

void OpenTeamCommunicator::observe(SequentialTimer& timer)
{
  // Update time data
  d_currentTime = Clock::getTimestamp();
  assert(d_lastBroadcast <= d_currentTime);

  // TODO period in config
  if (Clock::getSecondsSince(d_lastBroadcast) > 0.5)
  {
    // Send values using mitecom
    sendData();
  }
}

void OpenTeamCommunicator::receiveData()
{
  char buffer[BUFFER_SIZE];

  while (true)
  {
    // Receive a message (if available), this call is nonblocking
    ssize_t messageLength = mitecom_receive(d_sock, buffer, BUFFER_SIZE);

    if (messageLength <= 0)
      break;

    // Message received, update our view of the Team
    MixedTeamMate teamMate = MixedTeamParser::parseIncoming(buffer, messageLength, d_teamNumber);

    if (teamMate.robotID == d_uniformNumber)
      continue;

    // Update data on Teammate
    if (d_teamMates.find(teamMate.robotID) == d_teamMates.end())
      log::info("OpenTeamCommunicator::receiveData") << "First message seen from team mate " << teamMate.robotID;

    // Add team teamMate to our map
    d_teamMates[teamMate.robotID] = teamMate;

    // Remember the last time (i.e. now) that we heard from this robot
    d_teamMates[teamMate.robotID].lastUpdate = d_currentTime;

    // Make visible to State
    State::set(make_shared<OpenTeamState const>(d_teamMates));
  }
}

void OpenTeamCommunicator::sendData()
{
  // NOTE protocol uses millimeters, we use meters -- scale values

  auto const& agentFrameState = State::get<AgentFrameState>();
  auto const& worldFrameState = State::get<WorldFrameState>();

  // Get values to be sent
  auto const& ballObservation = agentFrameState->getBallObservation();
  auto const& agentPosition = worldFrameState->getPosition();

  MixedTeamMate myInformation;
  myInformation.robotID = d_uniformNumber;
  myInformation.data[ROBOT_CURRENT_ROLE] = ROLE_IN_TEAM; // TODO Check whether this is needed
  myInformation.data[ROBOT_ABSOLUTE_X] = uint(agentPosition.x() * 1000);
  myInformation.data[ROBOT_ABSOLUTE_Y] = uint(agentPosition.y() * 1000);
  myInformation.data[ROBOT_ABSOLUTE_ORIENTATION] = Math::radToDeg(agentPosition.theta());

  if (ballObservation)
  {
    myInformation.data[BALL_RELATIVE_X] = ballObservation->x() * 1000;
    myInformation.data[BALL_RELATIVE_Y] = ballObservation->y() * 1000;
  }

  // TODO Get bot's belief in its position

  uint32_t messageDataLength = 0;

  // Serialize and Broadcast Data
  auto messageData = unique_ptr<MixedTeamCommMessage>(MixedTeamParser::create(&messageDataLength, myInformation, d_teamNumber, d_uniformNumber));
  assert(messageData != nullptr);
  assert(messageDataLength > 0);

  mitecom_broadcast(d_sock, REMOTE_PORT, messageData.get(), messageDataLength);

  d_lastBroadcast = d_currentTime;
}

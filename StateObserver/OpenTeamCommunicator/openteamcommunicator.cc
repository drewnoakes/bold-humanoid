#include "openteamcommunicator.hh"

#include "../../Clock/clock.hh"
#include "../../Config/config.hh"
#include "../../Math/math.hh"
#include "../../State/state.hh"
#include "../../StateObject/OpenTeamState/openteamstate.hh"

#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using namespace std;
using namespace bold;
using namespace Eigen;

OpenTeamCommunicator::OpenTeamCommunicator(unsigned teamNumber, unsigned uniformNumber)
: StateObserver::StateObserver("Open Team Communicator", ThreadId::ThinkLoop),
  d_teamNumber(teamNumber),
  d_uniformNumber(uniformNumber),
  d_localPort(Config::getStaticValue<int>("mitecom.local-port")),
  d_remotePort(Config::getStaticValue<int>("mitecom.remote-port")),
  d_sendPeriodSeconds(Config::getSetting<double>("mitecom.send-period-seconds")),
  d_lastBroadcast(Clock::getTimestamp())
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
  // Update time data
  d_currentTime = Clock::getTimestamp();
  assert(d_lastBroadcast <= d_currentTime);

  if (Clock::getSecondsSince(d_lastBroadcast) > d_sendPeriodSeconds->getValue())
  {
    // Send values using mitecom
    sendData();
  }
}

void OpenTeamCommunicator::receiveData()
{
  // TODO this buffer size is probably way too large
  const uint BUFFER_SIZE = 32767;

  char buffer[BUFFER_SIZE];

  while (true)
  {
    // Receive a message (if available), this call is nonblocking
    ssize_t messageLength = mitecom_receive(d_sock, buffer, BUFFER_SIZE);

    if (messageLength <= 0)
      break;

    // Message received, update our view of the Team
    MixedTeamMate teamMate = MixedTeamParser::parseIncoming(buffer, messageLength, d_teamNumber);

    // Ignore messages from ourselves.
    if (teamMate.robotID == d_uniformNumber)
      continue;

    // An ID of -1 indicates that the message was invalid.
    if (teamMate.robotID == -1)
    {
      log::error("OpenTeamCommunicator::receiveData") << "Error parsing open team message";
      continue;
    }

    log::verbose("OpenTeamCommunicator::receiveData") << "Received message with length " << messageLength << " bytes from robot " << teamMate.robotID;

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
  myInformation.data[ROBOT_CURRENT_ROLE] = ROLE_OTHER; // TODO Check whether this is needed
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

  mitecom_broadcast(d_sock, d_remotePort, messageData.get(), messageDataLength);

  d_lastBroadcast = d_currentTime;
}

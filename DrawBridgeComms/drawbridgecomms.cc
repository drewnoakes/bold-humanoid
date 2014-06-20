#include "drawbridgecomms.hh"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "../Agent/agent.hh"
#include "../BehaviourControl/behaviourcontrol.hh"
#include "../Config/config.hh"
#include "../Debugger/debugger.hh"
#include "../Option/option.hh"
#include "../State/state.hh"
#include "../StateObject/AgentFrameState/agentframestate.hh"
#include "../StateObject/GameState/gamestate.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../StateObject/OptionTreeState/optiontreestate.hh"
#include "../StateObject/TeamState/teamstate.hh"
#include "../UDPSocket/udpsocket.hh"
#include "../version.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

DrawBridgeComms::DrawBridgeComms(Agent* agent, std::shared_ptr<BehaviourControl> behaviourControl, std::shared_ptr<Debugger> debugger)
: d_agent(agent),
  d_behaviourControl(behaviourControl),
  d_debugger(debugger),
  d_socket(make_unique<UDPSocket>())
{
  int port = Config::getStaticValue<int>("drawbridge.udp-port");

  d_socket->setBlocking(false);
//   d_socket->setMulticastLoopback(true);
  d_socket->setBroadcast(true);
  d_socket->setMulticastTTL(1);
  // TODO broadcast address in config
  d_socket->setTarget("255.255.255.255", port);
  if (d_socket->bind("", port))
    log::info("DrawBridgeComms") << "Bound to port " << port;
  else
    log::warning("DrawBridgeComms") << "Error binding to port " << port;
}

void DrawBridgeComms::publish()
{
  StringBuffer buffer;
  buildMessage(buffer);

  d_socket->send(buffer.GetString(), buffer.GetSize());

  if (d_debugger)
    d_debugger->notifySendingDrawbridgeMessage();
}

// TODO include: FPS (think/motion)
//               Memory usage

void DrawBridgeComms::buildMessage(StringBuffer& buffer)
{
  Writer<StringBuffer> writer(buffer);

  static int uniformNumber = Config::getStaticValue<int>("uniform-number");
  static int teamNumber = Config::getStaticValue<int>("team-number");

  writer.StartObject();
  {
    writer.String("unum").Int(uniformNumber);
    writer.String("team").Int(teamNumber);
    writer.String("ver").String(Version::GIT_SHA1.c_str());
    writer.String("uptime").Uint(static_cast<int>(d_agent->getUptimeSeconds()));

    writer.String("activity").String(getPlayerActivityString(d_behaviourControl->getPlayerActivity()).c_str());
    writer.String("role").String(getPlayerRoleString(d_behaviourControl->getPlayerRole()).c_str());
    writer.String("status").String(getPlayerStatusString(d_behaviourControl->getPlayerStatus()).c_str());

    auto agentFrame = State::get<AgentFrameState>();
    if (agentFrame)
    {
      writer.String("agent");
      writer.StartObject();
      {
        if (agentFrame->isBallVisible())
        {
          writer.String("ball")
            .StartArray()
            .Double(agentFrame->getBallObservation()->x())
            .Double(agentFrame->getBallObservation()->y())
            .EndArray();
        }
        if (agentFrame->goalObservationCount())
        {
          writer.String("goals").StartArray();
          for (auto const& goal : agentFrame->getGoalObservations())
            writer.StartArray().Double(goal.x()).Double(goal.y()).EndArray();
          writer.EndArray();
        }
      }
      writer.EndObject();
    }

    auto game = State::get<GameState>();
    if (game)
    {
      writer.String("game");
      writer.StartObject();
      {
        writer.String("mode").String(game->getPlayModeString().c_str());
        writer.String("age").Uint(static_cast<unsigned>(game->getAgeMillis()));
      }
      writer.EndObject();
    }

    auto hw = State::get<HardwareState>();
    if (hw)
    {
      writer.String("hw");
      writer.StartObject();
      {
        writer.String("volt").Double(hw->getCM730State().voltage);
        writer.String("power").Bool(hw->getCM730State().isPowered);
        writer.String("temps").StartArray();
        for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
          writer.Uint(hw->getMX28State(jointId).presentTemp);
        writer.EndArray();
      }
      writer.EndObject();
    }

    auto team = State::get<TeamState>();
    if (team)
    {
      writer.String("team");
      writer.StartArray();
      {
        for (PlayerState const& player : team->players())
        {
          if (player.isMe())
            continue;
          writer.StartObject();
          {
            writer.String("unum").Int(player.uniformNumber);
            writer.String("ms").Int(static_cast<int>(player.getAgeMillis()));
          }
          writer.EndObject();
        }
      }
      writer.EndArray();
    }

    auto optionTree = State::get<OptionTreeState>();
    if (optionTree)
    {
      writer.String("options");
      writer.StartArray();
      {
        for (auto const& option : optionTree->getRanOptions())
          writer.String(option->getId().c_str());
      }
      writer.EndArray();
    }
  }
  writer.EndObject();
}

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
#include "../StateObject/TimingState/timingstate.hh"
#include "../StateObject/TeamState/teamstate.hh"
#include "../UDPSocket/udpsocket.hh"
#include "../Version/version.hh"

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

  const int MaxHostLength = 30;
  char hostChars[MaxHostLength];
  if (gethostname(hostChars, MaxHostLength) != 0)
  {
    log::warning("DrawBridgeComms::DrawBridgeComms") << "Unable to determine hostname: " << strerror(errno) << " (" << errno << ")";
    d_hostName = "<unknown>";
  }
  else
  {
    d_hostName = hostChars;
  }
}

void DrawBridgeComms::publish()
{
  StringBuffer buffer;
  buildMessage(buffer);

  d_socket->send(buffer.GetString(), (int)buffer.GetSize());

  if (d_debugger)
    d_debugger->notifySendingDrawbridgeMessage();
}

// TODO include: Memory usage

void DrawBridgeComms::buildMessage(StringBuffer& buffer)
{
  Writer<StringBuffer> writer(buffer);

  static int uniformNumber = Config::getStaticValue<int>("uniform-number");
  static int teamNumber = Config::getStaticValue<int>("team-number");
  static int teamColour = Config::getStaticValue<int>("team-colour");
  static string playerName = Config::getStaticValue<string>("player-name");

  writer.StartObject();
  {
    writer.String("unum").Int(uniformNumber);
    writer.String("team").Int(teamNumber);
    writer.String("col").Int(teamColour);
    writer.String("host").String(d_hostName.c_str());
    writer.String("name").String(playerName.c_str());
    writer.String("ver").String(Version::GIT_SHA1.c_str());
    writer.String("built").String(Version::BUILT_ON_HOST_NAME.c_str());
    writer.String("uptime").Uint(static_cast<uint>(d_agent->getUptimeSeconds()));

    writer.String("activity").String(getPlayerActivityString(d_behaviourControl->getPlayerActivity()).c_str());
    writer.String("role").String(getPlayerRoleString(d_behaviourControl->getPlayerRole()).c_str());
    writer.String("status").String(getPlayerStatusString(d_behaviourControl->getPlayerStatus()).c_str());

    auto thinkTiming = State::get<ThinkTimingState>();
    auto motionTiming = State::get<MotionTimingState>();

    if (thinkTiming)
      writer.String("fpsThink").Double(thinkTiming->getAverageFps());
    if (motionTiming)
      writer.String("fpsMotion").Double(motionTiming->getAverageFps());

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
      writer.String("teammates");
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
      auto const& ranOptions = optionTree->getRanOptions();
      auto const& fsmStates = optionTree->getFSMStates();

      writer.String("options");
      writer.Array(
        ranOptions.begin(), ranOptions.end(),
        [&](shared_ptr<Option> const& option) { writer.String(option->getId().c_str()); });

      writer.String("fsms");
      writer.Array(
        fsmStates.begin(), fsmStates.end(),
        [&](FSMStateSnapshot const& fsmState) { writer.StartObject()
          .String("fsm").String(fsmState.getFsmName().c_str())
          .String("state").String(fsmState.getStateName().c_str())
          .EndObject();
        });
    }
  }
  writer.EndObject();
}

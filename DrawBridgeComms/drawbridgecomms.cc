#include "drawbridgecomms.hh"

#include <rapidjson/writer.h>

#include "../Agent/agent.hh"
#include "../BehaviourControl/behaviourcontrol.hh"
#include "../Config/config.hh"
#include "../MessageCounter/messagecounter.hh"
#include "../Option/option.hh"
#include "../State/state.hh"
#include "../StateObject/AgentFrameState/agentframestate.hh"
#include "../StateObject/GameState/gamestate.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../StateObject/OptionTreeState/optiontreestate.hh"
#include "../StateObject/TimingState/timingstate.hh"
#include "../Version/version.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

DrawBridgeComms::DrawBridgeComms(Agent* agent, shared_ptr<BehaviourControl> behaviourControl, shared_ptr<MessageCounter> messageCounter)
: d_agent(agent),
  d_behaviourControl(behaviourControl),
  d_messageCounter(messageCounter),
  d_socket(make_unique<UDPSocket>())
{
  int port = Config::getStaticValue<int>("drawbridge.udp-port");

  d_socket->setBlocking(false);
//   d_socket->setMulticastLoopback(true);
  d_socket->setBroadcast(true);
  d_socket->setMulticastTTL(1);
  // TODO broadcast address in config
  d_socket->setTarget("255.255.255.255", port);
  if (d_socket->bind(port))
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

  d_messageCounter->notifySendingDrawbridgeMessage();
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
    writer.String("unum");
    writer.Int(uniformNumber);
    writer.String("team");
    writer.Int(teamNumber);
    writer.String("col");
    writer.Int(teamColour);
    writer.String("host");
    writer.String(d_hostName.c_str());
    writer.String("name");
    writer.String(playerName.c_str());
    writer.String("ver");
    writer.String(Version::GIT_SHA1.c_str());
    writer.String("built");
    writer.String(Version::BUILT_ON_HOST_NAME.c_str());
    writer.String("uptime");
    writer.Uint(static_cast<uint>(d_agent->getUptimeSeconds()));

    writer.String("activity");
    writer.String(getPlayerActivityString(d_behaviourControl->getPlayerActivity()).c_str());
    writer.String("role");
    writer.String(getPlayerRoleString(d_behaviourControl->getPlayerRole()).c_str());
    writer.String("status");
    writer.String(getPlayerStatusString(d_behaviourControl->getPlayerStatus()).c_str());

    auto thinkTiming = State::get<ThinkTimingState>();
    auto motionTiming = State::get<MotionTimingState>();

    if (thinkTiming)
    {
      writer.String("fpsThink");
      writer.Double(thinkTiming->getAverageFps(), "%.3f");
    }

    if (motionTiming)
    {
      writer.String("fpsMotion");
      writer.Double(motionTiming->getAverageFps(), "%.3f");
    }

    auto agentFrame = State::get<AgentFrameState>();
    if (agentFrame)
    {
      writer.String("agent");
      writer.StartObject();
      {
        if (agentFrame->isBallVisible())
        {
          writer.String("ball");
          writer.StartArray();
          writer.Double(agentFrame->getBallObservation()->x(), "%.3f");
          writer.Double(agentFrame->getBallObservation()->y(), "%.3f");
          writer.EndArray();
        }
        if (agentFrame->goalObservationCount())
        {
          writer.String("goals");
          writer.StartArray();
          for (auto const& goal : agentFrame->getGoalObservations())
          {
            writer.StartArray();
            writer.Double(goal.x(), "%.3f");
            writer.Double(goal.y(), "%.3f");
            writer.EndArray();
          }
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
        writer.String("mode");
        writer.String(getPlayModeName(game->getPlayMode()).c_str());
        writer.String("age");
        writer.Uint(static_cast<unsigned>(game->getAgeMillis()));
      }
      writer.EndObject();
    }

    auto hw = State::get<HardwareState>();
    if (hw)
    {
      writer.String("hw");
      writer.StartObject();
      {
        writer.String("volt");
        writer.Double(hw->getCM730State().voltage, "%.1f");
        writer.String("power");
        writer.Bool(hw->getCM730State().isPowered);
        writer.String("temps");
        writer.StartArray();
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
            writer.String("unum");
            writer.Int(player.uniformNumber);
            writer.String("ms");
            writer.Int(static_cast<int>(player.getAgeMillis()));
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
      writer.StartArray();
      for (auto& option : ranOptions)
        writer.String(option->getId().c_str());
      writer.EndArray();

      writer.String("fsms");
      writer.StartArray();
      for (auto& fsmState : fsmStates)
      {
        writer.StartObject();
        writer.String("fsm");
        writer.String(fsmState.getFsmName().c_str());
        writer.String("state");
        writer.String(fsmState.getStateName().c_str());
        writer.EndObject();
      }
    }
  }
  writer.EndObject();
}

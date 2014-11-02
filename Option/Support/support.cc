#include "support.hh"

#include "../../MotionModule/WalkModule/walkmodule.hh"
#include "../WalkTo/walkto.hh"

#include "../../Config/config.hh"
#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../../Math/math.hh"

using namespace bold;
using namespace std;
using namespace Eigen;

Support::Support(string const& id, shared_ptr<WalkModule> walkModule)
  : Option{id, "Support"}
{
  d_yieldDistance = Config::getSetting<double>("options.support.yield-distance");
  d_walkTo = make_shared<WalkTo>(id + ".walkto", walkModule);
}

Option::OptionVector Support::runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer)
{
  auto agentFrame = State::get<AgentFrameState>();

  if (!agentFrame->isBallVisible())
    return {};

  Vector2d ballPos{agentFrame->getBallObservation()->head<2>()};
  auto ballDir = ballPos.normalized();

  writer.String("ballPos");
  writer.StartArray();
  writer.Double(ballPos.x(), "%.3f");
  writer.Double(ballPos.y(), "%.3f");
  writer.EndArray(2);

  auto yieldPos = ballPos - d_yieldDistance->getValue() * ballDir;

  double ballAngleRads = Math::angleToPoint(ballPos);

  d_walkTo->setTargetPosition(yieldPos, ballAngleRads);
  return {d_walkTo};
}

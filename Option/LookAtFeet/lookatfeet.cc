#include "lookatfeet.hh"

#include "../../Config/config.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"
#include "../../State/state.hh"
#include "../../StateObject/AgentFrameState/agentframestate.hh"
#include "../../util/log.hh"

using namespace bold;
using namespace Eigen;
using namespace std;
using namespace rapidjson;

LookAtFeet::LookAtFeet(std::string const& id, std::shared_ptr<HeadModule> headModule)
  : Option(id, "LookAtFeet"),
    d_avgBallPos(30),
    d_headModule(headModule)
{
  d_panDegs = Config::getSetting<double>("options.look-at-feet.head-pan-degs");
  d_tiltDegs = Config::getSetting<double>("options.look-at-feet.head-tilt-degs");
}

vector<shared_ptr<Option>> LookAtFeet::runPolicy(Writer<StringBuffer>& writer)
{
  d_headModule->moveToDegs(d_panDegs->getValue(), d_tiltDegs->getValue());

  auto ballObs = State::get<AgentFrameState>()->getBallObservation();

  if (ballObs)
  {
    log::verbose("LookAtFeet::runPolicy") << "Ball pos in agent frame " << ballObs->transpose();
    d_avgBallPos.next(*ballObs);
    writer.String("ball");
    writer.StartArray();
    writer.Double(ballObs->x(), "%.3f");
    writer.Double(ballObs->y(), "%.3f");
    writer.EndArray(2);
  }
  else
  {
    writer.String("ball");
    writer.Null();
  }

  return {};
}

Vector3d LookAtFeet::getAverageBallPositionAgentFrame() const
{
  if (d_avgBallPos.count() == 0)
  {
    log::error("LookAtFeet::getBallPositionAgentFrame") << "No ball observations available";
    throw runtime_error("No ball observations available");
  }

  return d_avgBallPos.getAverage();
}

#include "adhocoptiontreebuilder.hh"

#include "../../Drawing/drawing.hh"
#include "../../FieldMap/fieldmap.hh"
#include "../../MotionScript/motionscript.hh"
#include "../../Option/LocateBall/locateball.hh"
#include "../../Option/LookAround/lookaround.hh"
#include "../../Option/MotionScriptOption/motionscriptoption.hh"
#include "../../Option/StopWalking/stopwalking.hh"
#include "conditionals.hh"

using namespace bold;
using namespace bold::Colour;
using namespace Eigen;
using namespace std;

shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildPenaltyKeeperFsm(Agent* agent)
{
  // SCRIPTS

  auto diveLeftScript = MotionScript::fromFile("./motionscripts/dive-left.json");
  auto diveRightScript = MotionScript::fromFile("./motionscripts/dive-left.json");
//  auto diveSplitsScript = MotionScript::fromFile("./motionscripts/dive-splits.json");

  auto signalLeftScript = MotionScript::fromFile("./motionscripts/signal-left-side.json");
  auto signalRightScript = MotionScript::fromFile("./motionscripts/signal-right-side.json");
//  auto signalBothScript = MotionScript::fromFile("./motionscripts/signal-both-sides.json");

  // OPTIONS

  auto standUp = make_shared<MotionScriptOption>("stand-up-script", agent->getMotionScriptModule(), "./motionscripts/stand-ready-upright.json");
  auto diveLeft = make_shared<MotionScriptOption>("dive-left", agent->getMotionScriptModule());
  auto diveRight = make_shared<MotionScriptOption>("dive-right", agent->getMotionScriptModule());
//  auto diveSplits = make_shared<MotionScriptOption>("splits", agent->getMotionScriptModule());
  auto stopWalking = make_shared<StopWalking>("stop-walking", agent->getWalkModule());
  auto locateBall = make_shared<LocateBall>("locate-ball", agent, 80.0, LookAround::speedIfBallVisible(0.15, 0.5), 30, 5);

  // SETTINGS

  Config::getSetting<bool>("keeper.dive.indicate-only")->track([=](bool indicateOnly)
  {
    diveLeft->setMotionScript(indicateOnly ? signalLeftScript : diveLeftScript);
    diveRight->setMotionScript(indicateOnly ? signalRightScript : diveRightScript);
//    diveSplits->setMotionScript(indicateOnly ? signalBothScript : diveSplitsScript);
  });

  // STATES

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "penalty-keeper");

  auto standUpState = fsm->newState("stand-up", { standUp }, false/*endState*/, true/*startState*/);
  auto locateBallState = fsm->newState("locate-ball", { stopWalking, locateBall });
  auto leftDiveState = fsm->newState("left-dive", { diveLeft });
  auto rightDiveState = fsm->newState("right-dive", { diveRight });

  // TRANSITIONS

  standUpState
    ->transitionTo(locateBallState, "standing")
    ->whenTerminated();

  const static double goalWidth = FieldMap::getGoalY();
  static Bounds2d leftBallDiveArea(Vector2d(-goalWidth/1.5, -0.2), Vector2d(-0.1, 1.0));
  static Bounds2d rightBallDiveArea(Vector2d(0.1, -0.2), Vector2d(goalWidth/1.5, 1.0));

  locateBallState
    ->transitionTo(leftDiveState, "ball-left")
    ->when([]
    {
      return stepUpDownThreshold(3, []
      {
        Draw::fillPolygon(Frame::Agent, leftBallDiveArea, bgr::red, 0.3, bgr::black, 0.0, 0);
        auto ball = State::get<AgentFrameState>()->getBallObservation();
        return ball && leftBallDiveArea.contains(ball->head<2>());
      });
    });

  locateBallState
    ->transitionTo(rightDiveState, "ball-right")
    ->when([]
    {
      return stepUpDownThreshold(3, []
      {
        Draw::fillPolygon(Frame::Agent, rightBallDiveArea, bgr::red, 0.3, bgr::red, 0.0, 0);
        auto ball = State::get<AgentFrameState>()->getBallObservation();
        return ball && rightBallDiveArea.contains(ball->head<2>());
      });
    });

  leftDiveState
    ->transitionTo(locateBallState, "done")
    ->whenTerminated();

  rightDiveState
    ->transitionTo(locateBallState, "done")
    ->whenTerminated();

  return fsm;
}

#include "adhocoptiontreebuilder.hh"

#include "../../Option/SequenceOption/sequenceoption.hh"
#include "../../Option/StopWalking/stopwalking.hh"
#include "../../Option/MotionScriptOption/motionscriptoption.hh"
#include "../../Option/WaitForWhistle/waitforwhistle.hh"
#include "../../State/state.hh"
#include "conditionals.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

/// The robot will approach the ball then circle it
shared_ptr<FSMOption> AdHocOptionTreeBuilder::buildWhistleListenerFsm(Agent* agent)
{
  // OPTIONS

  auto stopWalking  = make_shared<StopWalking>("stop-walking", agent->getWalkModule());
  auto standStraight = make_shared<MotionScriptOption>("stand-straight", agent->getMotionScriptModule(), "./motionscripts/stand-straight.json");
  auto signal = make_shared<MotionScriptOption>("signal", agent->getMotionScriptModule(), "./motionscripts/signal-left-side.json");
  auto waitForWhistle = make_shared<WaitForWhistle>("wait-for-whistle");

  // STATES

  auto fsm = make_shared<FSMOption>(agent->getVoice(), "whistle-listener");

  auto standState = fsm->newState("stand", { SequenceOption::make("stop-and-stand", { stopWalking, standStraight }) }, false/*endState*/, true/*startState*/);
  auto waitForWhistleState = fsm->newState("wait-for-whistle", { waitForWhistle });
  auto signalState = fsm->newState("signal", { signal });

  // TRANSITIONS

  standState
    ->transitionTo(waitForWhistleState, "standing")
    ->whenTerminated();

  // start approaching the ball when we have the confidence that it's really there
  waitForWhistleState
    ->transitionTo(signalState, "heard")
    ->whenTerminated();

  // stop walking to ball once we're close enough
  signalState
    ->transitionTo(standState, "reset")
    ->after(chrono::seconds(2));

  return fsm;
}

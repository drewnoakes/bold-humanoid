#include "odometer.hh"

#include "../../util/log.hh"
#include "../../StateObject/AmbulatorState/ambulatorstate.hh"

#include <cassert>

using namespace bold;
using namespace Eigen;
using namespace std;

Odometer::Odometer(shared_ptr<WalkModule> walkModule)
: TypedStateObserver<BodyState>("Odometer", ThreadId::MotionLoop),
  d_walkModule(walkModule),
  d_lastBodyState(),
  d_progress(),
  d_progressMutex()
{
  assert(walkModule);
}

void Odometer::observeTyped(shared_ptr<BodyState const> state, SequentialTimer& timer)
{
  assert(state);

  auto ambulatorState = AgentState::get<AmbulatorState>();

  if (!ambulatorState || !ambulatorState->isRunning())
  {
    d_lastBodyState = nullptr;
    return;
  }

  if (d_lastBodyState)
  {
    // Measure delta of movement

    int phase = ambulatorState->getCurrentPhase();

    bool isLeftSupportFoot = phase == WalkModule::PHASE0 || phase == WalkModule::PHASE1;

    JointId supportFootJointId = isLeftSupportFoot
      ? JointId::L_ANKLE_ROLL
      : JointId::R_ANKLE_ROLL;

    // TODO this is only approximate -- translating like this doesn't account for rotation

    Vector3d thisTranslation = state          ->getJoint(supportFootJointId)->transform.translation();
    Vector3d lastTranslation = d_lastBodyState->getJoint(supportFootJointId)->transform.translation();

    lock_guard<mutex> lock(d_progressMutex);
    d_progress += (thisTranslation - lastTranslation);
  }

  d_lastBodyState = state;
}

Vector3d Odometer::flush()
{
  lock_guard<mutex> lock(d_progressMutex);
  auto val = d_progress;
  d_progress = {};
  return val;
}

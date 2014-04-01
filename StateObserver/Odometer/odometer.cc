#include "odometer.hh"

#include "../../Config/config.hh"
#include "../../StateObject/AmbulatorState/ambulatorstate.hh"
#include "../../StateObject/OdometryState/odometrystate.hh"
#include "../../util/log.hh"

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
  State::make<OdometryState>(d_progress);

  Config::addAction("odometer.reset", "Reset odometer", [this]()
  {
    lock_guard<mutex> lock(d_progressMutex);
    d_progress = {0,0,0};
  });
}

void Odometer::observeTyped(shared_ptr<BodyState const> const& state, SequentialTimer& timer)
{
  assert(state);

  auto ambulatorState = State::get<AmbulatorState>();

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

    State::make<OdometryState>(d_progress);
  }

  d_lastBodyState = state;
}

Vector3d Odometer::getTranslation() const
{
  lock_guard<mutex> lock(d_progressMutex);
  return d_progress;
}

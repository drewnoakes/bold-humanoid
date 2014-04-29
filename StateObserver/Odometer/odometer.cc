#include "odometer.hh"

#include "../../Config/config.hh"
#include "../../WalkEngine/walkengine.hh"
#include "../../StateObject/WalkState/walkstate.hh"
#include "../../StateObject/OdometryState/odometrystate.hh"
#include "../../util/assert.hh"
#include "../../util/log.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

Odometer::Odometer(shared_ptr<WalkModule> walkModule)
: TypedStateObserver<BodyState>("Odometer", ThreadId::MotionLoop),
  d_walkModule{walkModule},
  d_lastBodyState{},
  d_transform{Affine3d::Identity()}
{
  ASSERT(walkModule);
  State::make<OdometryState>(d_transform);

  Config::addAction("odometer.reset", "Reset odometer", [this]()
  {
    lock_guard<mutex> lock(d_transformMutex);
    d_transform = Affine3d::Identity();
  });
}

void Odometer::observeTyped(shared_ptr<BodyState const> const& state, SequentialTimer& timer)
{
  ASSERT(state);

  auto walkState = State::get<WalkState>();

  if (!walkState || !walkState->isRunning())
  {
    d_lastBodyState = nullptr;
    return;
  }

  if (d_lastBodyState)
  {
    // Measure delta of movement

    // AtA0 = AtAt-1 * At-1A0
    //      = AtFt * FtAt-1 * At-1A0
    //      = AtFt * FtFt-1 * Ft-1At-1 * At-1A0
    //      = AtFt * Ft-1At-1 * At-1A0
    auto leftFootAgentTr = state->determineFootAgentTr(true);
    auto rightFootAgentTr = state->determineFootAgentTr(false);

    // Translation is location of agent/torso iin foot frame, so stance/lowest foot has highest z
    bool isLeftSupportFoot = leftFootAgentTr.translation().z() > rightFootAgentTr.translation().z();

    auto lastFootAgentTr = d_lastBodyState->determineFootAgentTr(isLeftSupportFoot);
    auto agentFootTr = isLeftSupportFoot ? leftFootAgentTr.inverse() : rightFootAgentTr.inverse();

    lock_guard<mutex> lock(d_transformMutex);
    d_transform = agentFootTr * lastFootAgentTr * d_transform;

    State::make<OdometryState>(d_transform);
  }

  d_lastBodyState = state;
}

Eigen::Affine3d Odometer::getTransform() const {
  lock_guard<mutex> lock(d_transformMutex);
  return d_transform;
}

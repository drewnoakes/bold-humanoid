#include "walkmodule.hh"

#include "../../Config/config.hh"
#include "../../Math/math.hh"
#include "../../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../../Setting/setting.hh"
#include "../../State/state.hh"
#include "../../StateObject/WalkState/walkstate.hh"
#include "../../ThreadUtil/threadutil.hh"
#include "../../WalkEngine/walkengine.hh"
#include "../../util/assert.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

string bold::getWalkStatusName(WalkStatus status)
{
  switch (status)
  {
    case bold::WalkStatus::Stopped:     return "Stopped";
    case bold::WalkStatus::Starting:    return "Starting";
    case bold::WalkStatus::Walking:     return "Walking";
    case bold::WalkStatus::Stabilising: return "Stabilising";
    default: return "Unknown";
  }
}

WalkModule::WalkModule(shared_ptr<MotionTaskScheduler> scheduler)
: MotionModule("walk", scheduler),
  d_walkEngine(make_shared<WalkEngine>()),
  d_stabilisationTimeMillis(Config::getSetting<int>("walk-module.stabilisation-time-ms")),
  d_stabilisationCyclesRemaining(0),
  d_xAmp(0, 1),
  d_yAmp(0, 1),
  d_turnAmp(0, 1),
  d_maxHipPitchAtSpeed(Config::getSetting<double>("walk-module.max-hip-pitch-at-speed")),
  d_minHipPitch(Config::getSetting<double>("walk-module.min-hip-pitch")),
  d_maxHipPitch(Config::getSetting<double>("walk-module.max-hip-pitch")),
  d_fwdAccelerationHipPitchFactor(Config::getSetting<double>("walk-module.fwd-acc-hip-pitch-factor")),
  d_turnAngleSet(false),
  d_moveDirSet(false),
  d_immediateStopRequested(false),
  d_status(WalkStatus::Stopped)
{
  Config::getSetting<double>("walk-module.x-amp-delta")->track([this](double value) { d_xAmp.setDelta(value); });
  Config::getSetting<double>("walk-module.y-amp-delta")->track([this](double value) { d_yAmp.setDelta(value); });
  Config::getSetting<double>("walk-module.turn-delta") ->track([this](double value) { d_turnAmp.setDelta(value); });
}

void WalkModule::setMoveDir(double x, double y)
{
  if (x == 0 && y == 0 && d_status == WalkStatus::Stopped)
    return;

  if (x == d_xAmp.getTarget() && y == d_yAmp.getTarget())
    return;

  if (d_moveDirSet)
    log::error("WalkModule::setMoveDir") << "Movement direction set twice between calls to step";

  d_moveDirSet = true;
  d_xAmp.setTarget(x);
  d_yAmp.setTarget(y);

  if (d_status == WalkStatus::Stopped)
    start();
}

void WalkModule::setTurnAngle(double turnAngle)
{
  if (turnAngle == 0 && d_status == WalkStatus::Stopped)
    return;

  if (turnAngle == d_turnAmp.getTarget())
    return;

  if (d_turnAngleSet)
    log::error("WalkModule::setTurnAngle") << "Turn angle set twice between calls to step";

  d_turnAngleSet = true;
  d_turnAmp.setTarget(turnAngle);

  if (d_status == WalkStatus::Stopped)
    start();
}

void WalkModule::start()
{
  d_status = WalkStatus::Starting;
  getScheduler()->add(this,
                      Priority::Low,  Required::No,  RequestCommit::No,   // HEAD
                      Priority::High, Required::Yes, RequestCommit::Yes,  // ARMS
                      Priority::High, Required::Yes, RequestCommit::Yes); // LEGS
}

void WalkModule::stop()
{
  if (d_status == WalkStatus::Stopped)
    return;

  d_xAmp.setTarget(0);
  d_yAmp.setTarget(0);
  d_turnAmp.setTarget(0);
}

void WalkModule::stopImmediately()
{
  if (d_status == WalkStatus::Stopped)
    return;

  d_immediateStopRequested = true;
}

void WalkModule::step(std::shared_ptr<JointSelection> const& selectedJoints)
{
  ASSERT(ThreadUtil::isMotionLoopThread());
  ASSERT(d_stabilisationCyclesRemaining >= 0);

  if (d_status == WalkStatus::Stopped)
    return;

  if (d_immediateStopRequested)
  {
    // Hard set everything to zero
    d_xAmp.reset();
    d_yAmp.reset();
    d_turnAmp.reset();
    d_status = WalkStatus::Stopped;
    d_immediateStopRequested = false;

    // Indicate we no longer need to be committed
    setCompletedFlag();

    State::make<WalkState>(0, 0, 0, 0, 0, 0, this, d_walkEngine);

    return;
  }

  if (d_status == WalkStatus::Starting)
  {
    d_status = WalkStatus::Walking;
    d_walkEngine->reset();
  }

  double xAmpPrior = d_xAmp.getCurrent();
  double yAmpPrior = d_yAmp.getCurrent();
  double turnAmpPrior = d_turnAmp.getCurrent();

  double xAmp = d_xAmp.getNext();
  double yAmp = d_yAmp.getNext();
  double turnAmp = d_turnAmp.getNext();

  double xAmpDelta = xAmp - xAmpPrior;
  double yAmpDelta = yAmp - yAmpPrior;
  double turnAmpDelta = turnAmp - turnAmpPrior;

  d_turnAngleSet = false;
  d_moveDirSet = false;

  if (xAmp == 0 && yAmp == 0 && turnAmp == 0)
  {
    if (d_status == WalkStatus::Walking)
    {
      d_status = WalkStatus::Stabilising;
      d_stabilisationCycleCount = d_stabilisationTimeMillis->getValue() / TIME_UNIT;
      d_stabilisationCyclesRemaining = d_stabilisationCycleCount;
    }
    else if (d_status == WalkStatus::Stabilising)
    {
      if (d_stabilisationCyclesRemaining == 0)
      {
        if (d_walkEngine->canStopNow())
        {
          setCompletedFlag();
          d_status = WalkStatus::Stopped;
        }
      }
      else
      {
        d_stabilisationCyclesRemaining--;
      }
    }
    else
    {
      // This should never happen
      ASSERT(false && "Logic error");
    }
  }
  else
  {
    ASSERT(d_status == WalkStatus::Walking);

    d_walkEngine->X_MOVE_AMPLITUDE = xAmp;
    d_walkEngine->Y_MOVE_AMPLITUDE = yAmp;
    d_walkEngine->A_MOVE_AMPLITUDE = turnAmp;

    // TODO allow swappable implementations of a WalkPitchPosture, and calculate every cycle on the motion thread

    // TODO this doesn't support walking backwards (-ve x)
    // TODO examine using the acceleration (delta xAmp) as a input signal

    // Lerp hip angle based on forward speed, or turn speed (whichever is greatest)
//    // TODO revisit this treatment of xAmp and turnAmp as though they're the same units
//     double alpha = max(xAmp, turnAmp) / d_maxHipPitchAtSpeed->getValue();
    double alpha = xAmp / d_maxHipPitchAtSpeed->getValue();

    double xAmpTargetDiff = d_xAmp.getTarget() - xAmp;

    alpha += d_fwdAccelerationHipPitchFactor->getValue() * xAmpTargetDiff;

    d_walkEngine->HIP_PITCH_OFFSET = Math::lerp(
      Math::clamp(alpha, 0.0, 1.0),
      d_minHipPitch->getValue(),
      d_maxHipPitch->getValue());
  }

  if (d_status != WalkStatus::Stopped)
  {
    d_walkEngine->step();

    double balanceGain = d_status == WalkStatus::Stabilising
      ? (double)d_stabilisationCyclesRemaining / d_stabilisationCycleCount
      : 1.0;

    d_walkEngine->balance(balanceGain);
  }

  State::make<WalkState>(
    d_xAmp.getTarget(), d_yAmp.getTarget(), d_turnAmp.getTarget(),
    xAmpDelta, yAmpDelta, turnAmpDelta,
    this,
    d_walkEngine);
}

void WalkModule::applyHead(HeadSection* head) { d_walkEngine->applyHead(head); }
void WalkModule::applyArms(ArmSection* arms)  { d_walkEngine->applyArms(arms); }
void WalkModule::applyLegs(LegSection* legs)  { d_walkEngine->applyLegs(legs); }

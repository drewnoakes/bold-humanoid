#include "walkmodule.hh"

#include "../../Balance/balance.hh"
#include "../../Balance/GyroBalance/gyrobalance.hh"
#include "../../Balance/OrientationBalance/orientationbalance.hh"
#include "../../BodyControl/bodycontrol.hh"
#include "../../Config/config.hh"
#include "../../State/state.hh"
#include "../../StateObject/BalanceState/balancestate.hh"
#include "../../StateObject/WalkState/walkstate.hh"
#include "../../WalkEngine/walkengine.hh"

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
  d_xAmpSmoother(0, 1),
  d_yAmpSmoother(0, 1),
  d_turnAmpSmoother(0, 1),
  d_hipPitchSmoother(Config::getValue<double>("walk-module.hip-pitch.stable-angle"), 1),
  d_isParalysed(Config::getSetting<bool>("walk-module.is-paralysed")),
  d_stableHipPitch(Config::getSetting<double>("walk-module.hip-pitch.stable-angle")),
  d_minHipPitch(Config::getSetting<double>("walk-module.hip-pitch.min-angle")),
  d_maxHipPitch(Config::getSetting<double>("walk-module.hip-pitch.max-angle")),
  d_maxHipPitchAtSpeed(Config::getSetting<double>("walk-module.hip-pitch.max-at-fwd-speed")),
  d_fwdAccelerationHipPitchFactor(Config::getSetting<double>("walk-module.hip-pitch.fwd-acc-factor")),
  d_bwdAccelerationHipPitchFactor(Config::getSetting<double>("walk-module.hip-pitch.bwd-acc-factor")),
  d_turnAngleSet(false),
  d_moveDirSet(false),
  d_immediateStopRequested(false),
  d_status(WalkStatus::Stopped)
{
  Config::getSetting<double>("walk-module.smoothing-deltas.x-amp")->track([this](double value) { d_xAmpSmoother.setDelta(value); });
  Config::getSetting<double>("walk-module.smoothing-deltas.y-amp")->track([this](double value) { d_yAmpSmoother.setDelta(value); });
  Config::getSetting<double>("walk-module.smoothing-deltas.turn") ->track([this](double value) { d_turnAmpSmoother.setDelta(value); });
  Config::getSetting<double>("walk-module.smoothing-deltas.hip-pitch")->track([this](double value) { d_hipPitchSmoother.setDelta(value); });

  Config::getSetting<BalanceMode>("balance.mode")->track(
    [this] (BalanceMode mode)
    {
      switch (mode)
      {
        case BalanceMode::None:
          d_balance = nullptr;
          break;
        case BalanceMode::Gyro:
          d_balance = make_shared<GyroBalance>();
          break;
        case BalanceMode::Orientation:
          d_balance = make_shared<OrientationBalance>();
          break;
      }
    });
}

void WalkModule::setMoveDir(double x, double y)
{
  if (x == 0 && y == 0 && d_status == WalkStatus::Stopped)
    return;

  if (x == d_xAmpSmoother.getTarget() && y == d_yAmpSmoother.getTarget())
    return;

  if (d_moveDirSet)
    log::error("WalkModule::setMoveDir") << "Movement direction set twice between calls to step";

  d_moveDirSet = true;
  d_xAmpSmoother.setTarget(x);
  d_yAmpSmoother.setTarget(y);

  if (d_status == WalkStatus::Stopped)
    start();
}

void WalkModule::setTurnAngle(double turnAngle)
{
  if (turnAngle == 0 && d_status == WalkStatus::Stopped)
    return;

  if (turnAngle == d_turnAmpSmoother.getTarget())
    return;

  if (d_turnAngleSet)
    log::error("WalkModule::setTurnAngle") << "Turn angle set twice between calls to step";

  d_turnAngleSet = true;
  d_turnAmpSmoother.setTarget(turnAngle);

  if (d_status == WalkStatus::Stopped)
    start();
}

void WalkModule::start()
{
  d_status = WalkStatus::Starting;
  getScheduler()->request(
    this,
    Priority::Low,  Required::No,  RequestCommit::No,   // HEAD
    Priority::High, Required::Yes, RequestCommit::Yes,  // ARMS
    Priority::High, Required::Yes, RequestCommit::Yes); // LEGS
}

void WalkModule::stop()
{
  if (d_status == WalkStatus::Stopped)
    return;

  d_xAmpSmoother.setTarget(0);
  d_yAmpSmoother.setTarget(0);
  d_turnAmpSmoother.setTarget(0);
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
    d_xAmpSmoother.reset();
    d_yAmpSmoother.reset();
    d_turnAmpSmoother.reset();
    d_status = WalkStatus::Stopped;
    d_immediateStopRequested = false;

    // Indicate we no longer need to be committed
    setCompletedFlag();

    State::make<WalkState>(0, 0, 0, 0, 0, 0, 0, 0, this, d_walkEngine);

    return;
  }

  if (d_status == WalkStatus::Starting)
  {
    d_status = WalkStatus::Walking;
    d_walkEngine->reset();
  }

  double xAmpPrior = d_xAmpSmoother.getCurrent();
  double yAmpPrior = d_yAmpSmoother.getCurrent();
  double turnAmpPrior = d_turnAmpSmoother.getCurrent();

  double xAmp = d_xAmpSmoother.getNext();
  double yAmp = d_yAmpSmoother.getNext();
  double turnAmp = d_turnAmpSmoother.getNext();

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
      d_hipPitchSmoother.setTarget(d_stableHipPitch->getValue());
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
    // It may be that we've had new walking params set during the stabilisation phase
    if (d_status == WalkStatus::Stabilising)
      d_status = WalkStatus::Walking;

    ASSERT(d_status == WalkStatus::Walking);

    //
    // SET WALK MOVEMENT PARAMETERS
    //

    d_walkEngine->X_MOVE_AMPLITUDE = xAmp;
    d_walkEngine->Y_MOVE_AMPLITUDE = yAmp;
    d_walkEngine->A_MOVE_AMPLITUDE = turnAmp;

    //
    // SET HIP PITCH TARGET
    //

    // TODO allow swappable implementations of a WalkPitchPosture, and calculate every cycle on the motion thread
    // TODO this doesn't support walking backwards (-ve x)
//    // TODO revisit this treatment of xAmp and turnAmp as though they're the same units
//     double alpha = max(xAmp, turnAmp) / d_maxHipPitchAtSpeed->getValue();
    double alpha = xAmp / d_maxHipPitchAtSpeed->getValue();

//    // Estimate future forward acceleration by comparing the target forward speed with the current.
//    // Note that the target can fluctuate considerably, so this value may be quite noisy.
//    double xAcc = d_xAmpSmoother.getTarget() - xAmp;

    // The change in xAmp gives a direction and magnitude of our acceleration in the forward direction.
    double xAcc = xAmpDelta;

    if (xAcc > 0)
      alpha += d_fwdAccelerationHipPitchFactor->getValue() * xAcc;
    else
      alpha += d_bwdAccelerationHipPitchFactor->getValue() * xAcc;

    d_hipPitchSmoother.setTarget(Math::lerp(
      Math::clamp(alpha, 0.0, 1.0),
      d_minHipPitch->getValue(),
      d_maxHipPitch->getValue()));
  }

  //
  // UPDATE HIP PITCH
  //

  double hipPitchPrior = d_hipPitchSmoother.getCurrent();
  double hipPitch = d_hipPitchSmoother.getNext();
  double hipPitchDelta = hipPitch - hipPitchPrior;
  d_walkEngine->HIP_PITCH_OFFSET = hipPitch;

  if (d_status != WalkStatus::Stopped)
  {
    //
    // Calculate new motion
    //

    d_walkEngine->step();

    //
    // Calculate balance parameters
    //

    // Take a copy, for thread safety
    auto balance = d_balance;

    if (balance != nullptr)
      State::make<BalanceState>(balance->computeCorrection(Math::degToRad(d_hipPitchSmoother.getCurrent())));
    else
      State::set<BalanceState>(nullptr);
  }

  State::make<WalkState>(
    d_xAmpSmoother.getTarget(), d_yAmpSmoother.getTarget(), d_turnAmpSmoother.getTarget(), d_hipPitchSmoother.getTarget(),
    xAmpDelta, yAmpDelta, turnAmpDelta, hipPitchDelta,
    this,
    d_walkEngine);
}

void WalkModule::applyHead(HeadSection* head) { if (!d_isParalysed->getValue()) d_walkEngine->applyHead(head); }
void WalkModule::applyArms(ArmSection* arms)  { if (!d_isParalysed->getValue()) d_walkEngine->applyArms(arms); }

void WalkModule::applyLegs(LegSection* legs)
{
  if (!d_isParalysed->getValue())
    d_walkEngine->applyLegs(legs);

  auto balanceState = State::get<BalanceState>();

  if (balanceState)
  {
    double ratio = d_status == WalkStatus::Stabilising
      ? (double)d_stabilisationCyclesRemaining / d_stabilisationCycleCount
      : 1.0;

    auto const& correction = balanceState->offsets();

    legs->hipRollRight()->setModulationOffset(static_cast<short>(round(ratio * correction.hipRoll)));
    legs->hipRollLeft()->setModulationOffset(static_cast<short>(round(ratio * correction.hipRoll)));

    legs->kneeRight()->setModulationOffset(static_cast<short>(round(ratio * correction.knee)));
    legs->kneeLeft()->setModulationOffset(static_cast<short>(round(-ratio * correction.knee)));

    legs->anklePitchRight()->setModulationOffset(static_cast<short>(round(ratio * correction.anklePitch)));
    legs->anklePitchLeft()->setModulationOffset(static_cast<short>(round(-ratio * correction.anklePitch)));

    legs->ankleRollRight()->setModulationOffset(static_cast<short>(round(ratio * correction.ankleRoll)));
    legs->ankleRollLeft()->setModulationOffset(static_cast<short>(round(ratio * correction.ankleRoll)));
  }
}

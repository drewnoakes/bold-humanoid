#include "headmodule.hh"

#include "../../BodyControl/bodycontrol.hh"
#include "../../Config/config.hh"
#include "../../Math/math.hh"
#include "../../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../../State/state.hh"
#include "../../StateObject/BodyState/bodystate.hh"
#include "../../ThreadUtil/threadutil.hh"

using namespace bold;
using namespace std;

HeadModule::HeadModule(std::shared_ptr<MotionTaskScheduler> const& scheduler)
: MotionModule("head", scheduler)
{
  // P gain values for MX28
  d_gainP       = Config::getSetting<int>("head-module.p-gain");

  // PD gain values for tracking
  d_panGainP    = Config::getSetting<double>("head-module.tracking.pan.p-gain");
  d_panGainI    = Config::getSetting<double>("head-module.tracking.pan.i-gain");
  d_panILeak    = Config::getSetting<double>("head-module.tracking.pan.i-leak");
  d_panGainD    = Config::getSetting<double>("head-module.tracking.pan.d-gain");
  d_tiltGainP   = Config::getSetting<double>("head-module.tracking.tilt.p-gain");
  d_tiltGainI   = Config::getSetting<double>("head-module.tracking.tilt.i-gain");
  d_tiltILeak   = Config::getSetting<double>("head-module.tracking.tilt.i-leak");
  d_tiltGainD   = Config::getSetting<double>("head-module.tracking.tilt.d-gain");

  // Restrictions placed upon the range of movement by the head within this module
  d_limitPanDegs  = Config::getSetting<Range<double>>("head-module.pan-limit-degrees");
  d_limitTiltDegs = Config::getSetting<Range<double>>("head-module.tilt-limit-degrees");

  // Home position
  d_panHomeDegs   = Config::getSetting<double>("head-module.home-pan");
  d_tiltHomeDegs  = Config::getSetting<double>("head-module.home-tilt");

  auto fine = Config::getSetting<bool>("head-module.move-fine");

  // Controls
  Config::addAction("head-module.move-left", "&blacktriangleleft;",  [this,fine]() { fine->getValue() ? moveByDeltaDegs( 1, 0) : moveByDeltaDegs( 5, 0); });
  Config::addAction("head-module.move-up",   "&blacktriangle;",      [this,fine]() { fine->getValue() ? moveByDeltaDegs( 0, 1) : moveByDeltaDegs( 0, 5); });
  Config::addAction("head-module.move-down", "&blacktriangledown;",  [this,fine]() { fine->getValue() ? moveByDeltaDegs( 0,-1) : moveByDeltaDegs( 0,-5); });
  Config::addAction("head-module.move-right","&blacktriangleright;", [this,fine]() { fine->getValue() ? moveByDeltaDegs(-1, 0) : moveByDeltaDegs(-5, 0); });
  Config::addAction("head-module.move-home", "home",                 [this]() { moveToHome(); });
  Config::addAction("head-module.move-zero", "zero",                 [this]() { moveToDegs(0, 0); });

  initTracking();
}

void HeadModule::checkLimit()
{
  // Clamp pan/tilt within the box-shaped limit
  d_targetPanAngleDegs = d_limitPanDegs->getValue().clamp(d_targetPanAngleDegs);
  d_targetTiltAngleDegs = d_limitTiltDegs->getValue().clamp(d_targetTiltAngleDegs);

  // Lower corners of that box are disallowed as the head makes contact with the
  // shoulder, and the body occludes too much from the camera anyway.
  //
  //  -135           135
  //
  //     \           /  10
  //      \         /
  //       \_______/    -22
  //
  //      -85     85

  if (fabs(d_targetPanAngleDegs) > Math::degToRad(85))
  {
    // Outside of +/- 85 degrees, we need to prevent tilting too far down.
    // At 85 deg, we start tilting upwards to avoid the shoulder.
    double limit = Math::lerp(fabs(d_targetPanAngleDegs), 85, 135, -22, 10);
    if (d_targetTiltAngleDegs < limit)
      d_targetTiltAngleDegs = limit;
  }
}

void HeadModule::moveToHome()
{
  moveToDegs(d_panHomeDegs->getValue(), d_tiltHomeDegs->getValue());
}

void HeadModule::moveToDegs(double pan, double tilt)
{
  d_targetPanAngleDegs = pan;
  d_targetTiltAngleDegs = tilt;

  getScheduler()->add(this,
                      Priority::Normal, false,  // HEAD   Interuptable::YES
                      Priority::None,   false,  // ARMS
                      Priority::None,   false); // LEGS

  checkLimit();
}

void HeadModule::moveByDeltaDegs(double panDelta, double tiltDelta)
{
  auto body = State::get<BodyState>();
  double currentPanAngleDegs = Math::radToDeg(body->getJoint(JointId::HEAD_PAN)->angleRads);
  double currentTiltAngleDegs = Math::radToDeg(body->getJoint(JointId::HEAD_TILT)->angleRads);
  moveToDegs(currentPanAngleDegs + panDelta, currentTiltAngleDegs + tiltDelta);
}

void HeadModule::initTracking()
{
  d_lastPanError = 0;
  d_lastTiltError = 0;
  d_integratedPanError = 0;
  d_integratedTiltError = 0;
}

void HeadModule::moveTracking(double panError, double tiltError)
{
  double panErrorDelta = panError - d_lastPanError;
  double tiltErrorDelta = tiltError - d_lastTiltError;
  d_integratedPanError += -d_panILeak->getValue() * d_integratedPanError + panError;
  d_integratedTiltError += -d_tiltILeak->getValue() * d_integratedTiltError + tiltError;

  d_lastPanError = panError;
  d_lastTiltError = tiltError;

  auto calcPIDOffset = [](double error, double integratedError, double errorDelta, double p, double i, double d)
  {
    double pOffset = p * error;
    double iOffset = i * integratedError;
    double dOffset = d * errorDelta;

    return pOffset + iOffset + dOffset;
  };

  auto body = State::get<BodyState>();
  double currentPanAngleDegs = Math::radToDeg(body->getJoint(JointId::HEAD_PAN)->angleRads);
  double currentTiltAngleDegs = Math::radToDeg(body->getJoint(JointId::HEAD_TILT)->angleRads);

  d_targetPanAngleDegs =
    currentPanAngleDegs +
    calcPIDOffset(panError, d_integratedPanError, panErrorDelta,
                  d_panGainP->getValue(), d_panGainI->getValue(), d_panGainD->getValue());
  d_targetTiltAngleDegs =
    currentTiltAngleDegs +
    calcPIDOffset(tiltError, d_integratedTiltError, tiltErrorDelta,
                  d_tiltGainP->getValue(), d_tiltGainI->getValue(), d_tiltGainD->getValue());

  getScheduler()->add(this,
                      Priority::Normal, false,  // HEAD   Interuptable::YES
                      Priority::None,   false,  // ARMS
                      Priority::None,   false); // LEGS

  checkLimit();
}

void HeadModule::step(shared_ptr<JointSelection> const& selectedJoints)
{
  ASSERT(ThreadUtil::isMotionLoopThread());
  // TODO implement a head movement that updates its target position every 8ms instead of every 30ms, for smoother movements
}

void HeadModule::applyHead(HeadSection* head)
{
  auto gainP = d_gainP->getValue();
  head->visitJoints([this,gainP](JointControl* joint) { joint->setPGain(gainP); });

  head->pan()->setDegrees(d_targetPanAngleDegs);
  head->tilt()->setDegrees(d_targetTiltAngleDegs);
}

void HeadModule::applyArms(ArmSection* arms) { log::error("HeadModule::applyArms") << "SHOULD NOT BE CALLED"; }
void HeadModule::applyLegs(LegSection* legs) { log::error("HeadModule::applyLegs") << "SHOULD NOT BE CALLED"; }

#include "headmodule.hh"

#include "../AgentState/agentstate.hh"
#include "../BodyControl/bodycontrol.hh"
#include "../Config/config.hh"
#include "../Math/math.hh"
#include "../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../ThreadId/threadid.hh"

#include <iostream>

using namespace bold;
using namespace std;

HeadModule::HeadModule(std::shared_ptr<MotionTaskScheduler> scheduler)
: MotionModule("head", scheduler)
{
  // P gain values for MX28
  d_gainP       = Config::getSetting<int>("head-module.p-gain");

  // PD gain values for tracking
  d_panGainP    = Config::getSetting<double>("head-module.tracking.pan.p-gain");
  d_panGainD    = Config::getSetting<double>("head-module.tracking.pan.d-gain");
  d_tiltGainP   = Config::getSetting<double>("head-module.tracking.tilt.p-gain");
  d_tiltGainD   = Config::getSetting<double>("head-module.tracking.tilt.d-gain");

  // Restrictions placed upon the range of movement by the head within this module
  d_limitPanDegs  = Config::getSetting<Range<double>>("head-module.pan-limit-degrees");
  d_limitTiltDegs = Config::getSetting<Range<double>>("head-module.tilt-limit-degrees");

  // Home position
  d_panHomeDegs   = Config::getSetting<double>("head-module.home-pan");
  d_tiltHomeDegs  = Config::getSetting<double>("head-module.home-tilt");

  auto fine = Config::getSetting<bool>("head-module.move-fine");

  // Controls
  Config::addAction("head-module.move-left", "&blacktriangleleft;",  [this,fine]() { fine->getValue() ? moveByDeltaDegs( 0.5, 0.0) : moveByDeltaDegs( 5, 0); });
  Config::addAction("head-module.move-up",   "&blacktriangle;",      [this,fine]() { fine->getValue() ? moveByDeltaDegs( 0.0, 0.5) : moveByDeltaDegs( 0, 5); });
  Config::addAction("head-module.move-down", "&blacktriangledown;",  [this,fine]() { fine->getValue() ? moveByDeltaDegs( 0.0,-0.5) : moveByDeltaDegs( 0,-5); });
  Config::addAction("head-module.move-right","&blacktriangleright;", [this,fine]() { fine->getValue() ? moveByDeltaDegs(-0.5, 0.0) : moveByDeltaDegs(-5, 0); });
  Config::addAction("head-module.move-home", "home",                 [this]() { moveToHome(); });
  Config::addAction("head-module.move-zero", "zero",                 [this]() { moveToDegs(0, 0); });
}

HeadModule::~HeadModule()
{}

void HeadModule::checkLimit()
{
  // Clamp pan/tilt within the box-shaped limit
  d_panAngleDegs = d_limitPanDegs->getValue().clamp(d_panAngleDegs);
  d_tiltAngleDegs = d_limitTiltDegs->getValue().clamp(d_tiltAngleDegs);

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

  if (fabs(d_panAngleDegs) > Math::degToRad(85))
  {
    // Outside of +/- 85 degrees, we need to prevent tilting too far down.
    // At 85 deg, we start tilting upwards to avoid the shoulder.
    double limit = Math::lerp(fabs(d_panAngleDegs), 85, 135, -22, 10);
    if (d_tiltAngleDegs < limit)
      d_tiltAngleDegs = limit;
  }
}

void HeadModule::initialize()
{
  d_panAngleDegs = d_panHomeDegs->getValue();
  d_tiltAngleDegs = d_tiltHomeDegs->getValue();
  checkLimit();
  initTracking();
  moveToHome();
}

void HeadModule::moveToHome()
{
  moveToDegs(d_panHomeDegs->getValue(), d_tiltHomeDegs->getValue());
}

void HeadModule::moveToDegs(double pan, double tilt)
{
  d_panAngleDegs = pan;
  d_tiltAngleDegs = tilt;

  getScheduler()->add(this,
                      Priority::Normal, false,  // HEAD   Interuptable::YES
                      Priority::None,   false,  // ARMS
                      Priority::None,   false); // LEGS

  checkLimit();
}

void HeadModule::moveByDeltaDegs(double panDelta, double tiltDelta)
{
  moveToDegs(d_panAngleDegs + panDelta, d_tiltAngleDegs + tiltDelta);
}

void HeadModule::initTracking()
{
  d_lastPanError = 0;
  d_lastTiltError = 0;
}

void HeadModule::moveTracking(double panError, double tiltError)
{
  double panErrorDelta = panError - d_lastPanError;
  double tiltErrorDelta = tiltError - d_lastTiltError;

  d_lastPanError = panError;
  d_lastTiltError = tiltError;

  auto calcPDOffset = [](double error, double errorDelta, double p, double d)
  {
    double pOffset = pow(error * p, 2);
    if (error < 0)
      pOffset = -pOffset;

    double dOffset = pow(errorDelta * d, 2);
    if (errorDelta < 0)
      dOffset = -dOffset;

    return pOffset + dOffset;
  };

  d_panAngleDegs  += calcPDOffset(panError,  panErrorDelta,  d_panGainP->getValue(),  d_panGainD->getValue());
  d_tiltAngleDegs += calcPDOffset(tiltError, tiltErrorDelta, d_tiltGainP->getValue(), d_tiltGainD->getValue());

  getScheduler()->add(this,
                      Priority::Normal, false,  // HEAD   Interuptable::YES
                      Priority::None,   false,  // ARMS
                      Priority::None,   false); // LEGS

  checkLimit();
}

void HeadModule::step(shared_ptr<JointSelection> selectedJoints)
{
  assert(ThreadId::isMotionLoopThread());
  // TODO implement a head movement that updates its target position every 8ms instead of every 30ms, for smoother movements
}

void HeadModule::applyHead(std::shared_ptr<HeadSection> head)
{
  auto gainP = d_gainP->getValue();
  head->visitJoints([this,gainP](shared_ptr<JointControl> joint) { joint->setPGain(gainP); });

  head->pan()->setDegrees(d_panAngleDegs);
  head->tilt()->setDegrees(d_tiltAngleDegs);
}

void HeadModule::applyArms(std::shared_ptr<ArmSection> arms) { log::error("HeadModule::applyArms") << "SHOULD NOT BE CALLED"; }
void HeadModule::applyLegs(std::shared_ptr<LegSection> legs) { log::error("HeadModule::applyLegs") << "SHOULD NOT BE CALLED"; }

#include "headmodule.hh"

#include "../AgentState/agentstate.hh"
#include "../BodyControl/bodycontrol.hh"
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
  d_gainP    = getParam("p_gain", 8);

  // PD gain values for tracking
  d_panGainP    = getParam("tracking_pan_p_gain", 0.1);
  d_panGainD    = getParam("tracking_pan_d_gain", 0.22);
  d_tiltGainP   = getParam("tracking_tilt_p_gain", 0.1);
  d_tiltGainD   = getParam("tracking_tilt_d_gain", 0.22);

  // Restrictions placed upon the range of movement by the head within this module
  d_limitLeft   = getParam("left_limit", 100);
  d_limitRight  = getParam("right_limit", -100);
  d_limitTop    = getParam("top_limit", EYE_TILT_OFFSET_ANGLE);
  d_limitBottom = getParam("bottom_limit", EYE_TILT_OFFSET_ANGLE - 65);

  // Home position
  d_panHome     = getParam("pan_home", 0.0);
  d_tiltHome    = getParam("tilt_home", EYE_TILT_OFFSET_ANGLE - 30.0);

  // Controls
  d_controls.push_back(Control::createAction("&blacktriangleleft;",  [this]() { moveByDeltaDegs( 5, 0); }));
  d_controls.push_back(Control::createAction("&blacktriangle;",      [this]() { moveByDeltaDegs( 0, 5); }));
  d_controls.push_back(Control::createAction("&blacktriangledown;",  [this]() { moveByDeltaDegs( 0,-5); }));
  d_controls.push_back(Control::createAction("&blacktriangleright;", [this]() { moveByDeltaDegs(-5, 0); }));
  d_controls.push_back(Control::createAction("home",                 [this]() { moveToHome(); }));
  d_controls.push_back(Control::createAction("zero",                 [this]() { moveToDegs(0, 0); }));

  auto createControl = [this](double* target, string name, double min, double max, int scale = 1, bool isAdvanced = true)
  {
    auto control = Control::createInt(name, [target,scale]() { return int((*target)*scale); }, [this,target,scale](int value) { *target = value/double(scale); });
    control.setLimitValues(min * scale, max * scale);
    control.setIsAdvanced(isAdvanced);
    d_controls.push_back(control);
  };

  createControl(&d_gainP, "P Gain", 0, 32);

  createControl(&d_panGainP, "Tracking Pan P Gain", 0, 0.20, 100);
  createControl(&d_panGainD, "Tracking Pan D Gain", 0, 0.40, 100);
  createControl(&d_tiltGainP, "Tracking Tilt P Gain", 0, 0.20, 100);
  createControl(&d_tiltGainD, "Tracking Tilt D Gain", 0, 0.40, 100);

  createControl(&d_limitLeft, "Pan Limit Left", 1, 150);
  createControl(&d_limitRight, "Pan Limit Right", -150, -1);

  // No controls for top/bottom -- cannot go lower without damaging head, and no need to look upwards
}

HeadModule::~HeadModule()
{}

constexpr double HeadModule::EYE_TILT_OFFSET_ANGLE;

void HeadModule::checkLimit()
{
  d_panAngle = Math::clamp(d_panAngle, d_limitRight, d_limitLeft);
  d_tiltAngle = Math::clamp(d_tiltAngle, d_limitBottom, d_limitTop);
}

void HeadModule::initialize()
{
  d_panAngle = d_panHome;
  d_tiltAngle = d_tiltHome;
  checkLimit();
  initTracking();
  moveToHome();
}

void HeadModule::moveToHome()
{
  moveToDegs(d_panHome, d_tiltHome);
}

void HeadModule::moveToDegs(double pan, double tilt)
{
  d_panAngle = pan;
  d_tiltAngle = tilt;

  getScheduler()->add(this,
                      Priority::Normal, false,  // HEAD   Interuptable::YES
                      Priority::None,   false,  // ARMS
                      Priority::None,   false); // LEGS

  checkLimit();
}

void HeadModule::moveByDeltaDegs(double panDelta, double tiltDelta)
{
  moveToDegs(d_panAngle + panDelta, d_tiltAngle + tiltDelta);
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

  d_panAngle  += calcPDOffset(panError,  panErrorDelta,  d_panGainP,  d_panGainD);
  d_tiltAngle += calcPDOffset(tiltError, tiltErrorDelta, d_tiltGainP, d_tiltGainD);

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
  head->visitJoints([this](shared_ptr<JointControl> joint) { joint->setPGain(d_gainP); });

  head->pan()->setDegrees(d_panAngle);
  head->tilt()->setDegrees(d_tiltAngle);
}

void HeadModule::applyArms(std::shared_ptr<ArmSection> arms) { cerr << "[HeadModule::applyArms] SHOULD NOT BE CALLED" << endl; }
void HeadModule::applyLegs(std::shared_ptr<LegSection> legs) { cerr << "[HeadModule::applyLegs] SHOULD NOT BE CALLED" << endl; }

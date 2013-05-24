#include "headmodule.hh"

#include "../AgentState/agentstate.hh"
#include "../BodyControl/bodycontrol.hh"
#include "../Math/math.hh"
#include "../MotionTaskScheduler/motiontaskscheduler.hh"

#include <iostream>

using namespace bold;
using namespace std;

HeadModule::HeadModule(std::shared_ptr<MotionTaskScheduler> scheduler)
: MotionModule("head", scheduler)
{
  d_panGainP    = getParam("pan_p_gain", 0.1);
  d_panGainD    = getParam("pan_d_gain", 0.22);

  d_tiltGainP   = getParam("tilt_p_gain", 0.1);
  d_tiltGainD   = getParam("tilt_d_gain", 0.22);

  // Restrictions placed upon the range of movement by the head within this module
  d_limitLeft   = getParam("left_limit", 70);
  d_limitRight  = getParam("right_limit", -70);
  d_limitTop    = getParam("top_limit", EYE_TILT_OFFSET_ANGLE);
  d_limitBottom = getParam("bottom_limit", EYE_TILT_OFFSET_ANGLE - 65);

  d_panHome     = getParam("pan_home", 0.0);
  d_tiltHome    = getParam("tilt_home", EYE_TILT_OFFSET_ANGLE - 30.0);
}

HeadModule::~HeadModule()
{}

constexpr double HeadModule::EYE_TILT_OFFSET_ANGLE;

void HeadModule::checkLimit()
{
  d_panAngle = Math::clamp(d_panAngle, d_limitRight, d_limitLeft);
  d_tiltAngle = Math::clamp(d_panAngle, d_limitBottom, d_limitTop);
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
  moveToAngle(d_panHome, d_tiltHome);
}

void HeadModule::moveToAngle(double pan, double tilt)
{
  d_panAngle = pan;
  d_tiltAngle = tilt;
  
  getScheduler()->add(this,
                      Priority::Normal, false,  // HEAD   Interuptable::YES
                      Priority::None,   false,  // ARMS
                      Priority::None,   false); // LEGS

  checkLimit();
}

void HeadModule::moveByAngleOffset(double panDelta, double tiltDelta)
{
  moveToAngle(d_panAngle + panDelta, d_tiltAngle + tiltDelta);
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
  // TODO implement a head movement that updates its target position every 8ms instead of every 30ms, for smoother movements
}

void HeadModule::applyHead(std::shared_ptr<HeadSection> head)
{
  head->pan()->setAngle(d_panAngle);
  head->tilt()->setAngle(d_tiltAngle);
}

void HeadModule::applyArms(std::shared_ptr<ArmSection> arms) { cerr << "[HeadModule::applyArms] SHOULD NOT BE CALLED" << endl; }
void HeadModule::applyLegs(std::shared_ptr<LegSection> legs) { cerr << "[HeadModule::applyLegs] SHOULD NOT BE CALLED" << endl; }

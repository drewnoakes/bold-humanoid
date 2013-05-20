#include "headmodule.hh"

#include "../Math/math.hh"
#include "../minIni/minIni.h"
#include "../BodyControl/bodycontrol.hh"
#include "../AgentState/agentstate.hh"

#include <iostream>

using namespace bold;
using namespace std;

HeadModule::HeadModule(minIni const& ini)
{
  d_panGainP    = ini.getd("Head Module", "pan_p_gain", 0.1);
  d_panGainD    = ini.getd("Head Module", "pan_d_gain", 0.22);

  d_tiltGainP   = ini.getd("Head Module", "tilt_p_gain", 0.1);
  d_tiltGainD   = ini.getd("Head Module", "tilt_d_gain", 0.22);

  // Restrictions placed upon the range of movement by the head within this module
  d_limitLeft   = ini.getd("Head Module", "left_limit", 70);
  d_limitRight  = ini.getd("Head Module", "right_limit", -70);
  d_limitTop    = ini.getd("Head Module", "top_limit", EYE_TILT_OFFSET_ANGLE);
  d_limitBottom = ini.getd("Head Module", "bottom_limit", EYE_TILT_OFFSET_ANGLE - 65);

  d_panHome     = ini.getd("Head Module", "pan_home", 0.0);
  d_tiltHome    = ini.getd("Head Module", "tilt_home", EYE_TILT_OFFSET_ANGLE - 30.0);
}

HeadModule::~HeadModule()
{}

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

  checkLimit();
}

void HeadModule::moveByAngleOffset(double panDelta, double tiltDelta)
{
  moveToAngle(d_panAngle + panDelta, d_tiltAngle + tiltDelta);
}

void HeadModule::initTracking()
{
  d_panError = 0;
  d_tiltError = 0;
}

void HeadModule::moveTracking(double panError, double tiltError)
{
  double panErrorDelta = panError - d_panError;
  double tiltErrorDelta = tiltError - d_tiltError;

  d_panError = panError;
  d_tiltError = tiltError;

  auto calcDelta = [](double error, double errorDelta, double p, double d)
  {
    double pOffset = pow(error * p, 2);
    if (error < 0)
      pOffset = -pOffset;

    double dOffset = pow(errorDelta * d, 2);
    if (errorDelta < 0)
      dOffset = -dOffset;

    return pOffset + dOffset;
  };

  d_panAngle  += calcDelta(panError,  panErrorDelta,  d_panGainP,  d_panGainD);
  d_tiltAngle += calcDelta(tiltError, tiltErrorDelta, d_tiltGainP, d_tiltGainD);

  checkLimit();
}

void HeadModule::step(JointSelection const& selectedJoints)
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

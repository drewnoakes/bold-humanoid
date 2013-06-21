#include "ambulator.hh"

#include "../MotionModule/WalkModule/walkmodule.hh"

using namespace bold;

Ambulator::Ambulator(std::shared_ptr<WalkModule> walkModule)
  : Configurable("ambulator"),
    d_walkModule(walkModule),
    d_xAmp(0.0, getParam("xAmpDelta", 3.0)),
    d_yAmp(0.0, getParam("yAmpDelta", 3.0)),
    d_turnAmp(0.0, getParam("turnDelta", 1.0)),
    d_maxHipPitchAtSpeed(getParam("maxHipPitchAtSpeed", 15.0)),
    d_minHipPitch(getParam("minHipPitch", 13.0)),
    d_maxHipPitch(getParam("maxHipPitch", 17.0)),
    d_turnAngleSet(false),
    d_moveDirSet(false),
    d_controls()
{
  // TODO these should be double controls
  d_controls.push_back(Control::createInt("Min hip pitch",         [this]() { return d_minHipPitch; },        [this](int value) { d_minHipPitch = value; }));
  d_controls.push_back(Control::createInt("Max hip pitch",         [this]() { return d_maxHipPitch; },        [this](int value) { d_maxHipPitch = value; }));
  d_controls.push_back(Control::createInt("Max hip pitch @ speed", [this]() { return d_maxHipPitchAtSpeed; }, [this](int value) { d_maxHipPitchAtSpeed = value; }));
  d_controls.push_back(Control::createInt("X smoothing delta",     [this]() { return d_xAmp.getDelta(); },    [this](int value) { d_xAmp.setDelta(value); }));
  d_controls.push_back(Control::createInt("Y smoothing delta",     [this]() { return d_yAmp.getDelta(); },    [this](int value) { d_yAmp.setDelta(value); }));
  d_controls.push_back(Control::createInt("Turn smoothing delta",  [this]() { return d_turnAmp.getDelta(); }, [this](int value) { d_turnAmp.setDelta(value); }));

  d_controls.push_back(Control::createBool("Enable auto-balance",  [this]() { return d_walkModule->BALANCE_ENABLE; }, [this](bool value) { d_walkModule->BALANCE_ENABLE = value; }));
}

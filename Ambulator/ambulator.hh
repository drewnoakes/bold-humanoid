#pragma once

#include <Eigen/Core>
#include <memory>

#include "../Smoother/LinearSmoother/linearsmoother.hh"
#include "../Control/control.hh"
#include "../Configurable/configurable.hh"

namespace bold
{
  class WalkModule;

  class Ambulator : public Configurable
  {
  public:
    Ambulator(std::shared_ptr<WalkModule> walkModule)
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
      d_controls.push_back(Control::createInt("Min hip pitch", d_minHipPitch, [this](int value) { d_minHipPitch = value; }));
      d_controls.push_back(Control::createInt("Max hip pitch", d_maxHipPitch, [this](int value) { d_maxHipPitch = value; }));
      d_controls.push_back(Control::createInt("Max hip pitch @ speed", d_maxHipPitchAtSpeed, [this](int value) { d_maxHipPitchAtSpeed = value; }));
      d_controls.push_back(Control::createInt("X smoothing delta", d_xAmp.getDelta(), [this](int value) { d_xAmp.setDelta(value); }));
      d_controls.push_back(Control::createInt("Y smoothing delta", d_yAmp.getDelta(), [this](int value) { d_yAmp.setDelta(value); }));
      d_controls.push_back(Control::createInt("Turn smoothing delta", d_turnAmp.getDelta(), [this](int value) { d_turnAmp.setDelta(value); }));
    }

    void step();

    /**
     * Cause all motion to come to a halt. The walk will be stopped using
     * smoothing.
     */
    void stop()
    {
      d_xAmp.setTarget(0);
      d_yAmp.setTarget(0);
      d_turnAmp.setTarget(0);
    }

    bool isRunning() const;

    /**
     * Set the direction of motion, where positive X is in the forwards
     * direction, and positive Y is to the right. The length of the vector
     * determines the velocity of motion (unspecfied units).
     */
    void setMoveDir(Eigen::Vector2d const& moveDir);

    /**
     * Set the rate of turning, where positive values turn right (clockwise)
     * and negative values turn left (counter-clockwise) (unspecfied units).
     */
    void setTurnAngle(double turnSpeed);
    
    std::vector<Control> getControls() const { return d_controls; }

  private:
    std::shared_ptr<WalkModule> d_walkModule;
    LinearSmoother d_xAmp;
    LinearSmoother d_yAmp;
    LinearSmoother d_turnAmp;
    double d_maxHipPitchAtSpeed;
    double d_minHipPitch;
    double d_maxHipPitch;
    bool d_turnAngleSet;
    bool d_moveDirSet;
    std::vector<Control> d_controls;
  };
}

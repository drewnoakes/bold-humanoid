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
    Ambulator(std::shared_ptr<WalkModule> walkModule);

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

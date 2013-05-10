#pragma once

#include <Eigen/Core>
#include <memory>

#include "../Smoother/LinearSmoother/linearsmoother.hh"
#include "../minIni/minIni.h"

namespace bold
{
  class Walking;

  class Ambulator
  {
  public:
    Ambulator(std::shared_ptr<Walking> walkModule, minIni const& ini)
    : d_walkModule(walkModule),
      d_xAmp(0.0, ini.getd("Ambulator", "XAmpDelta", 3.0)),
      d_yAmp(0.0, ini.getd("Ambulator", "YAmpDelta", 3.0)),
      d_turnAmp(0.0, ini.getd("Ambulator", "TurnDelta", 1.0)),
      d_turnAngleSet(false),
      d_moveDirSet(false)
    {}

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

  private:
    std::shared_ptr<Walking> d_walkModule;
    LinearSmoother d_xAmp;
    LinearSmoother d_yAmp;
    LinearSmoother d_turnAmp;
    bool d_turnAngleSet;
    bool d_moveDirSet;
  };
}

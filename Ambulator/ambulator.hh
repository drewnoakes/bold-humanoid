#ifndef BOLD_AMBULATOR_HH
#define BOLD_AMBULATOR_HH

#include <Eigen/Core>
#include <LinuxDARwIn.h>
#include <LinuxCM730.h>
#include <iostream>

#include "../robotis/Framework/include/Walking.h"
#include "../Smoother/smoother.hh"
#include "../Smoother/LinearSmoother/linearsmoother.hh"

namespace bold
{
  class Ambulator
  {
  private:
    LinearSmoother d_xAmp;
    LinearSmoother d_yAmp;
    LinearSmoother d_turnAmp;

  public:
    Ambulator(minIni const& ini)
    : d_xAmp(0.0, ini.getd("Ambulator", "XAmpDelta", 3.0)),
      d_yAmp(0.0, ini.getd("Ambulator", "YAmpDelta", 3.0)),
      d_turnAmp(0.0, ini.getd("Ambulator", "TurnDelta", 1.0))
    {}

    void step()
    {
      auto walk = Robot::Walking::GetInstance();

      double xAmp = d_xAmp.getNext();
      double yAmp = d_yAmp.getNext();

      double turnAmp = d_turnAmp.getNext();

      if (xAmp == 0 && yAmp == 0 && turnAmp == 0)
      {
        if (walk->IsRunning())
        {
          std::cout << "[Ambulator] Stopping Walker" << std::endl;
          walk->Stop();
        }
      }
      else
      {
        std::cout << "[Ambulator] xAmp=" << xAmp << " yAmp=" << yAmp << " turnAmp=" << turnAmp << std::endl;

        walk->X_MOVE_AMPLITUDE = xAmp;
        walk->Y_MOVE_AMPLITUDE = yAmp;
        walk->A_MOVE_AMPLITUDE = turnAmp;

        if (!walk->IsRunning())
        {
          std::cout << "[Ambulator] Starting Walker" << std::endl;
          walk->Start();
          walk->m_Joint.SetEnableBodyWithoutHead(true, true);
        }
      }
    }

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

    /**
     * Set the direction of motion, where positive X is in the forwards
     * direction, and positive Y is to the right. The length of the vector
     * determines the velocity of motion (unspecfied units).
     */
    void setMoveDir(Eigen::Vector2d const& moveDir)
    {
      d_xAmp.setTarget(moveDir.x());
      d_yAmp.setTarget(moveDir.y());
    }

    /**
     * Set the rate of turning, where positive values turn right (clockwise)
     * and negative values turn left (counter-clockwise) (unspecfied units).
     */
    void setTurnAngle(double turnSpeed)
    {
      d_turnAmp.setTarget(turnSpeed);
    }
  };
}

#endif

#ifndef BOLD_AMBULATOR_HH
#define BOLD_AMBULATOR_HH

#include <Eigen/Core>
#include <LinuxDARwIn.h>
#include <LinuxCM730.h>

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
    Ambulator()
      : d_xAmp(0, 0.3),
        d_yAmp(0, 0.3),
        d_turnAmp(0, 1)
    {
    }

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
          walk->Stop();
        }
      }
      else
      {
        walk->X_MOVE_AMPLITUDE = xAmp;
        walk->Y_MOVE_AMPLITUDE = yAmp;
        walk->A_MOVE_AMPLITUDE = turnAmp;

        if (!walk->IsRunning())
        {
          walk->Start();
          walk->m_Joint.SetEnableBodyWithoutHead(true, true);
        }
      }
    }

    void stop()
    {
      d_xAmp.setTarget(0);
      d_yAmp.setTarget(0);
      d_turnAmp.setTarget(0);
    }

    void setMoveDir(Eigen::Vector2d moveDir)
    {
      d_xAmp.setTarget(moveDir.x());
      d_yAmp.setTarget(moveDir.y());
    }

    void setTurnAngle(double turnAngle)
    {
      d_turnAmp.setTarget(turnAngle);
    }
  };
}

#endif
#include "ambulator.ih"

void Ambulator::step()
{
  double xAmp = d_xAmp.getNext();
  double yAmp = d_yAmp.getNext();

  double turnAmp = d_turnAmp.getNext();

  if (xAmp == 0 && yAmp == 0 && turnAmp == 0)
  {
    if (d_walkModule->isRunning())
    {
      d_walkModule->stop();
    }
  }
  else
  {
    d_walkModule->X_MOVE_AMPLITUDE = xAmp;
    d_walkModule->Y_MOVE_AMPLITUDE = yAmp;
    d_walkModule->A_MOVE_AMPLITUDE = turnAmp;
    
    // TODO this doesn't support walking backwards (-ve x)
    // TODO examine using the acceleration (delta xAmp) as a input signal
    
    double alpha = Math::clamp(xAmp/d_maxHipPitchAtSpeed, 0.0, 1.0);

    d_walkModule->HIP_PITCH_OFFSET = Math::lerp(alpha, d_minHipPitch, d_maxHipPitch);

    if (!d_walkModule->isRunning())
    {
      // TODO this will look more sane when we make Ambulator the MotionModule, and have a generic WalkEngine with RobotisWalkEngine
      d_walkModule->getScheduler()->add(d_walkModule.get(),
                                        Priority::Optional,  true,  // HEAD   Interuptable::YES
                                        Priority::Important, true,  // ARMS
                                        Priority::Important, true); // LEGS
      d_walkModule->start();
    }
  }

  d_turnAngleSet = false;
  d_moveDirSet = false;

  AgentState::getInstance().set(make_shared<AmbulatorState const>(
    d_xAmp.getTarget(), d_yAmp.getTarget(), d_turnAmp.getTarget(), d_walkModule));
}

#include "ambulator.ih"

void Ambulator::step()
{
  assert(ThreadUtil::isThinkLoopThread());

  double xAmpPrior = d_xAmp.getCurrent();
  double yAmpPrior = d_yAmp.getCurrent();
  double turnAmpPrior = d_turnAmp.getCurrent();

  double xAmp = d_xAmp.getNext();
  double yAmp = d_yAmp.getNext();
  double turnAmp = d_turnAmp.getNext();

  double xAmpDelta = xAmp - xAmpPrior;
  double yAmpDelta = yAmp - yAmpPrior;
  double turnAmpDelta = turnAmp - turnAmpPrior;

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

    // TODO allow swappable implementations of a WalkPitchPosture, and calculate every cycle on the motion thread

    // TODO this doesn't support walking backwards (-ve x)
    // TODO examine using the acceleration (delta xAmp) as a input signal

    // Lerp hip angle based on forward speed, or turn speed (whichever is greatest)
//    // TODO revisit this treatment of xAmp and turnAmp as though they're the same units
//     double alpha = max(xAmp, turnAmp) / d_maxHipPitchAtSpeed->getValue();
    double alpha = xAmp / d_maxHipPitchAtSpeed->getValue();

    double xAmpTargetDiff = d_xAmp.getTarget() - xAmp;

    alpha += d_fwdAccelerationHipPitchFactor->getValue() * xAmpTargetDiff;

    d_walkModule->HIP_PITCH_OFFSET = Math::lerp(
      Math::clamp(alpha, 0.0, 1.0),
      d_minHipPitch->getValue(),
      d_maxHipPitch->getValue());

    if (!d_walkModule->isRunning())
    {
      // TODO this will look more sane when we make Ambulator the MotionModule, and have a generic WalkEngine with RobotisWalkEngine
      d_walkModule->getScheduler()->add(d_walkModule.get(),
                                        Priority::Optional,  true,   // HEAD   Interuptable::YES
                                        Priority::Important, false,  // ARMS
                                        Priority::Important, false); // LEGS
      d_walkModule->start();
    }
  }

  d_turnAngleSet = false;
  d_moveDirSet = false;

  State::make<AmbulatorState>(
    d_xAmp.getTarget(), d_yAmp.getTarget(), d_turnAmp.getTarget(),
    xAmpDelta, yAmpDelta, turnAmpDelta,
    d_walkModule);
}

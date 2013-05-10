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
//       cout << "[Ambulator] Stopping Walker" << endl;
      d_walkModule->stop();
    }
  }
  else
  {
    d_walkModule->X_MOVE_AMPLITUDE = xAmp;
    d_walkModule->Y_MOVE_AMPLITUDE = yAmp;
    d_walkModule->A_MOVE_AMPLITUDE = turnAmp;

//     d_walkModule->d_jointData.setEnableBodyWithoutHead(true, true);

    if (!d_walkModule->isRunning())
    {
//       cout << "[Ambulator] Starting Walker" << endl;
      d_walkModule->start();
    }
  }

  d_turnAngleSet = false;
  d_moveDirSet = false;

  AgentState::getInstance().set(make_shared<AmbulatorState const>(
    d_xAmp.getTarget(), d_yAmp.getTarget(), d_turnAmp.getTarget(), d_walkModule));
}
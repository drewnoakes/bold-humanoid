#include "ambulator.ih"

void Ambulator::step()
{
  auto walk = Walking::GetInstance();

  double xAmp = d_xAmp.getNext();
  double yAmp = d_yAmp.getNext();

  double turnAmp = d_turnAmp.getNext();

  if (xAmp == 0 && yAmp == 0 && turnAmp == 0)
  {
    if (walk->IsRunning())
    {
//       cout << "[Ambulator] Stopping Walker" << endl;
      walk->Stop();
    }
  }
  else
  {
    walk->X_MOVE_AMPLITUDE = xAmp;
    walk->Y_MOVE_AMPLITUDE = yAmp;
    walk->A_MOVE_AMPLITUDE = turnAmp;

    walk->m_Joint.SetEnableBodyWithoutHead(true, true);

    if (!walk->IsRunning())
    {
//       cout << "[Ambulator] Starting Walker" << endl;
      walk->Start();
    }
  }

  d_turnAngleSet = false;
  d_moveDirSet = false;

  AgentState::getInstance().set(make_shared<AmbulatorState const>(
    d_xAmp.getTarget(), d_yAmp.getTarget(), d_turnAmp.getTarget(), walk));
}
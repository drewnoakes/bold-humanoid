#include "agent.ih"

void Agent::lookForGoal()
{
  stand();

  int nObs = AgentState::getInstance().cameraFrame()->getGoalObservations().size();

  cout << "[Agent::lookForGoal] Visible goal count: " << nObs << endl;

  if (nObs >= 2)
    d_goalSeenCnt++;
  else if (d_goalSeenCnt > 0)
    d_goalSeenCnt--;

  static double phase = 0;

  if (nObs < 2)
  {
     // Oscillate
    double maxAmpH = 70.0;//d_ini.getd("Head Pan/Tilt", "left_limit", 80.0);
    double maxAmpV = 30.0;//
    //timeval tval;
    //gettimeofday(&tval, 0);

    //double t = tval.tv_sec + tval.tv_usec / 1e6;

    phase += 0.03/6 * 2 * M_PI;//(nObs == 1 ? 0.03 / 12 : 0.03 / 6.0) * 2 * M_PI;
    //double periodV = 1.4;

    float hAngle = sin(phase) * maxAmpH;
    //float vAngle = (sin(t / periodV * 2.0 * M_PI) + 1.0) * maxAmpV / 2.0;

    Head::GetInstance()->MoveByAngle(hAngle, maxAmpV);
    return;
  }

  if (d_goalSeenCnt >= 15)
  {
    lookAtGoal();

    d_state = State::S_START_CIRCLE_BALL;
  }
}

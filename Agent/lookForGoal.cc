#include "agent.ih"

#include <iterator>

void Agent::lookForGoal()
{
  int nObs = 0;

  for (Observation const& obs : d_observations)
  {
    if (obs.type == O_GOAL_POST)
      ++nObs;
  }

  cout << "nObs: " << nObs << endl;

  static double phase = 0;
  
  if (nObs < 2)
  {
     // Oscillate
    double maxAmpH = 70.0;//d_ini.getd("Head Pan/Tilt", "left_limit", 80.0);
    double maxAmpV = 30.0;//
    timeval tval;
    gettimeofday(&tval, 0);
    
    double t = tval.tv_sec + tval.tv_usec / 1e6;

    static auto w = d_camera.get(CV_CAP_PROP_FRAME_WIDTH);
    static auto h = d_camera.get(CV_CAP_PROP_FRAME_HEIGHT);

    phase += 0.03/6 * 2 * M_PI;//(nObs == 1 ? 0.03 / 12 : 0.03 / 6.0) * 2 * M_PI;
    double periodV = 1.4;

    float hAngle = sin(phase) * maxAmpH;
    float vAngle = (sin(t / periodV * 2.0 * M_PI) + 1.0) * maxAmpV / 2.0;

    Head::GetInstance()->MoveByAngle(hAngle, maxAmpV);
    return;
  }

  lookAtGoal();

  //d_state = S_CIRCLE_BALL;

}

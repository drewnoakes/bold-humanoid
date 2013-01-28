#include "agent.ih"

void Agent::circleBall()
{
  static Debugger::timestamp_t circleStartTime = d_debugger.getTimestamp();
  static double circleDurationSeconds;

  if (d_state == S_START_CIRCLE_BALL)
  {
    circleStartTime = d_debugger.getTimestamp();

    lookAtGoal();

    if (d_goalObservations.size() < 2)
    {
      d_goalSeenCnt--;
      if (d_goalSeenCnt <= 0)
      {
        d_goalSeenCnt = 0;
        d_state = S_LOOK_FOR_GOAL;
        return;
      }
    }

    double panAngle = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_PAN);
    double panAngleRange = Head::GetInstance()->GetLeftLimitAngle();
    double panRatio = panAngle / panAngleRange;

    circleDurationSeconds = panRatio * 0.4;

    printf("[Agent::circleBall] panRatio: %.3f circleDurationSeconds: %.1f\n", panRatio, circleDurationSeconds);

    if (abs(panRatio) < 0.1)
    {
      d_state = S_START_PREKICK_LOOK;
      return;
    }

    double x = -2;
    double y = panRatio < 0 ? 50 : -50;
    double a = panRatio < 0 ? -20 : 20;

    d_ambulator.setMoveDir(Eigen::Vector2d(x, y));
    d_ambulator.setTurnAngle(a);

    d_state = S_CIRCLE_BALL;
  }
  else
  {
    double dt = Debugger::getSeconds(circleStartTime);

    cout << "t circling: " << dt << endl;

    if (dt >= circleDurationSeconds)
    {
      stand();
      d_state = S_LOOK_FOR_GOAL;
    }
  }
}

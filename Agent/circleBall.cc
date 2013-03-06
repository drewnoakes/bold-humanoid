#include "agent.ih"

void Agent::circleBall()
{
  static Debugger::timestamp_t circleStartTime = Debugger::getTimestamp();
  static double circleDurationSeconds;

  auto& vision = VisualCortex::getInstance();

  if (d_state == S_START_CIRCLE_BALL)
  {
    circleStartTime = Debugger::getTimestamp();

    lookAtGoal();

    if (vision.goalObservations().size() < 2)
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

    if (abs(panRatio) < 0.2)
    {
      d_state = S_START_PREKICK_LOOK;
      return;
    }

    double x = d_circleBallX;
    double y = panRatio < 0 ? d_circleBallY : -d_circleBallY;
    double a = panRatio < 0 ? -d_circleBallTurn : d_circleBallTurn;

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

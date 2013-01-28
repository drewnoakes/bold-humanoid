#include "agent.ih"

void Agent::circleBall()
{
  lookAtGoal();

  double panAngle = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_PAN);
  double panAngleRange = Head::GetInstance()->GetLeftLimitAngle();
  double panRatio = panAngle / panAngleRange;

  printf("[Agent::circleBall] panRatio: %.3f\n", panRatio);

  if (abs(panRatio) < 0.05)
  {
    return;
  }

  double x = -2;
  double y = panRatio < 0 ? 50 : -50;
  double a = panRatio < 0 ? -20 : 20;

  d_ambulator.setMoveDir(Eigen::Vector2d(x, y));
  d_ambulator.setTurnAngle(a);
}

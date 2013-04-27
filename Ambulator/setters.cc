#include "ambulator.ih"

bool Ambulator::isRunning() const { return Walking::GetInstance()->IsRunning(); }

void Ambulator::setMoveDir(Eigen::Vector2d const& moveDir)
{
  if (d_moveDirSet)
    cerr << "[Ambulator::d_moveDirSet] Movement direction set twice between calls to step" << endl;
  d_moveDirSet = true;
  d_xAmp.setTarget(moveDir.x());
  d_yAmp.setTarget(moveDir.y());
}

void Ambulator::setTurnAngle(double turnSpeed)
{
  if (d_turnAngleSet)
    cerr << "[Ambulator::setTurnAngle] Turn angle set twice between calls to step" << endl;
  d_turnAngleSet = true;
  d_turnAmp.setTarget(turnSpeed);
}

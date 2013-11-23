#include "ambulator.ih"

bool Ambulator::isRunning() const { return d_walkModule->isRunning(); }

void Ambulator::setMoveDir(Eigen::Vector2d const& moveDir)
{
  if (d_moveDirSet)
    cerr << ccolor::error << "[Ambulator::d_moveDirSet] Movement direction set twice between calls to step" << ccolor::reset << endl;
  d_moveDirSet = true;
  d_xAmp.setTarget(moveDir.x());
  d_yAmp.setTarget(moveDir.y());
}

void Ambulator::setTurnAngle(double turnSpeed)
{
  if (d_turnAngleSet)
    cerr << ccolor::error << "[Ambulator::setTurnAngle] Turn angle set twice between calls to step" << ccolor::reset << endl;
  d_turnAngleSet = true;
  d_turnAmp.setTarget(turnSpeed);
}

#include "ambulator.ih"

bool Ambulator::isRunning() const { return d_walkModule->isRunning(); }

void Ambulator::setMoveDir(Eigen::Vector2d const& moveDir)
{
  if (d_moveDirSet)
    log::error("Ambulator::setMoveDir") << "Movement direction set twice between calls to step";
  
  d_moveDirSet = true;
  d_xAmp.setTarget(moveDir.x());
  d_yAmp.setTarget(moveDir.y());
}

void Ambulator::setTurnAngle(double turnSpeed)
{
  if (d_turnAngleSet)
    log::error("Ambulator::setTurnAngle") << "Turn angle set twice between calls to step";

  d_turnAngleSet = true;
  d_turnAmp.setTarget(turnSpeed);
}

#include "stopwalking.ih"

vector<shared_ptr<Option>> StopWalking::runPolicy(Writer<StringBuffer>& writer)
{
  d_walkModule->setMoveDir(Eigen::Vector2d(0,0));
  d_walkModule->setTurnAngle(0);

  writer.String("walkRunning").Bool(d_walkModule->isRunning());

  return {};
}

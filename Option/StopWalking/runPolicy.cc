#include "stopwalking.ih"

vector<shared_ptr<Option>> StopWalking::runPolicy(Writer<StringBuffer>& writer)
{
  d_ambulator->setMoveDir(Eigen::Vector2d(0,0));
  d_ambulator->setTurnAngle(0);

  return std::vector<std::shared_ptr<Option>>();
}

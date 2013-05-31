#include "stopwalking.ih"

std::vector<std::shared_ptr<Option>> StopWalking::runPolicy()
{
//   WalkModule::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);

  d_ambulator->setMoveDir(Eigen::Vector2d(0,0));
  d_ambulator->setTurnAngle(0);

  return std::vector<std::shared_ptr<Option>>();
}

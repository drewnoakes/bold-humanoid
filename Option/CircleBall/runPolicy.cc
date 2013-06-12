#include "circleball.ih"

std::vector<std::shared_ptr<Option>> CircleBall::runPolicy()
{
  auto bodyState = AgentState::get<BodyState>();

  double panAngle = bodyState->getHeadPanJoint()->angle;
  // TODO don't get this information from the head module, but rather some static model of the body's limits
  double panAngleRange = d_headModule->getLeftLimitRads();
  double panRatio = panAngle / panAngleRange;

  double x = 1;
  double y = panRatio < 0 ? 20 : -20;
  double a = panRatio < 0 ? -15 : 15;

  d_ambulator->setMoveDir(Eigen::Vector2d(x, y));
  d_ambulator->setTurnAngle(a);

  return std::vector<std::shared_ptr<Option>>();
}

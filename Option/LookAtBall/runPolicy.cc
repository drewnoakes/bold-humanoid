#include "lookatball.ih"

OptionList LookAtBall::runPolicy()
{
  auto const& ballObs = AgentState::getInstance().cameraFrame()->getBallObservation();

  if (!ballObs.hasValue())
  {
    cerr << "[LookAtBall::runPolicy] No ball seen" << endl;
    return OptionList();
  }

  auto ballPos = ballObs.value();

  static float r = 0.85;

  unsigned w = d_cameraModel->imageWidth();
  unsigned h = d_cameraModel->imageHeight();

  static Vector2f centerPx = Vector2f(w,h) / 2;
  static float happ = d_cameraModel->rangeHorizontalDegs() / w;
  static float vapp = d_cameraModel->rangeVerticalDegs() / h;

  Vector2f offset = (*ballPos - centerPx) * r;
  
  offset.x() *= happ; // pixel per angle
  offset.y() *= vapp; // pixel per angle

  offset = offset.cwiseMin(Vector2f(10,10)).cwiseMax(Vector2f(-10,-10));
  if (offset.norm() < 10)
    offset = Vector2f(0,0);

  Robot::Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
  Robot::Head::GetInstance()->MoveTracking(Robot::Point2D(offset.x(), offset.y()));

  return OptionList();
}

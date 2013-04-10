#include "lookatball.ih"

OptionPtr LookAtBall::runPolicy()
{
  auto const& ballObs = AgentState::getInstance().cameraFrame()->getBallObservation();

  if (!ballObs.hasValue())
  {
    cerr << "[LookAtBall::runPolicy] No ball seen" << endl;
    return 0;
  }

  auto ballPos = ballObs.value();

  cout << "Ball observation: " << (*ballPos).transpose() << endl;

  static float r = 0.85;

  // TODO get these from somewhere central
  static auto w = 320;//d_camera->getPixelFormat().width;
  static auto h = 240;//d_camera->getPixelFormat().height;

  static Vector2f centerPx = Vector2f(w,h) / 2;
  static float happ = 60.0 / w;//Camera::VIEW_H_ANGLE / w;
  static float vapp = 46.0 / h;//Camera::VIEW_V_ANGLE / h;

  Vector2f offset = (*ballPos - centerPx) * r;

  offset.x() *= happ; // pixel per angle
  offset.y() *= vapp; // pixel per angle

  offset = offset.cwiseMin(Vector2f(10,10)).cwiseMax(Vector2f(-10,-10));

  cout << "Looking at: " << offset.transpose() << endl;

  Robot::Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
  Robot::Head::GetInstance()->MoveTracking(Robot::Point2D(offset.x(), offset.y()));

  return 0;
}

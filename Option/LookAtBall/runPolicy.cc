#include "lookatball.ih"

OptionPtr LookAtBall::runPolicy()
{
  auto& vision = VisualCortex::getInstance();

  if (!vision.isBallVisible())
  {
    cerr << "[LookAtBall::runPolicy] No ball seen" << endl;
    return 0;
  }

  Vector2f lookAtPos = vision.ballObservation().pos;
  static float r = 0.85;

  // TODO get these from somewhere central
  static auto w = 320;//d_camera->getPixelFormat().width;
  static auto h = 240;//d_camera->getPixelFormat().height;

  static Vector2f centerPx = Vector2f(w,h) / 2;
  static float happ = 60 / w;//Camera::VIEW_H_ANGLE / w;
  static float vapp = 46 / h;//Camera::VIEW_V_ANGLE / h;

  Vector2f offset = (lookAtPos - centerPx) * r;

  offset.x() *= happ; // pixel per angle
  offset.y() *= vapp; // pixel per angle

  offset = offset.cwiseMin(Vector2f(10,10)).cwiseMax(Vector2f(-10,-10));

  Robot::Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
  Robot::Head::GetInstance()->MoveTracking(Robot::Point2D(offset.x(), offset.y()));

  return 0;
}

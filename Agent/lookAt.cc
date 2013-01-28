#include "agent.ih"

void Agent::lookAt(Vector2f const& pos)
{
  static float r = 0.85;

  static auto w = d_camera.get(CV_CAP_PROP_FRAME_WIDTH);
  static auto h = d_camera.get(CV_CAP_PROP_FRAME_HEIGHT);

  static Vector2f centerPx = Vector2f(w,h) / 2;
  static float happ = Camera::VIEW_H_ANGLE / w;
  static float vapp = Camera::VIEW_V_ANGLE / h;

  Vector2f offset = (pos - centerPx) * r;

  offset.x() *= happ; // pixel per angle
  offset.y() *= vapp; // pixel per angle

  offset = offset.cwiseMin(Vector2f(10,10)).cwiseMax(Vector2f(-10,-10));

  Head::GetInstance()->MoveTracking(Point2D(offset.x(), offset.y()));
}

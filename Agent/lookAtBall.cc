#include "agent.ih"

void Agent::lookAtBall()
{
  auto ballObs = find_if(d_observations.begin(), d_observations.end(),
			 [](Observation const& obs) { return obs.type == O_BALL; });

  bool ballSeen = ballObs != d_observations.end();
  if (!ballSeen)
  {
    cout << "Look at ball, but no ball seen" << endl;
    return;
  }

  Vector2f foundBallAtPx = ballObs->pos;
  
  float r = 0.85;

  static auto w = d_camera.get(CV_CAP_PROP_FRAME_WIDTH);
  static auto h = d_camera.get(CV_CAP_PROP_FRAME_HEIGHT);

  static Vector2f centerPx = Vector2f(w,h) / 2;
  static float happ = Camera::VIEW_H_ANGLE / w;
  static float vapp = Camera::VIEW_V_ANGLE / h;

  cout << "center: " << centerPx.transpose() << endl;

  Vector2f offset = ((foundBallAtPx - centerPx) * r).array();

  offset.x() *= happ; // pixel per angle
  offset.y() *= vapp; // pixel per angle

  cout << "offset 1: " << offset.transpose() << endl;

  offset = offset.cwiseMin(Vector2f(10,10)).cwiseMax(Vector2f(-10,-10));

  cout << "offset 2: " << offset.transpose() << endl;

  Head::GetInstance()->MoveTracking(Point2D(offset.x(), offset.y()));
  
}

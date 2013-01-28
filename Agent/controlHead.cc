#include "agent.ih"

void Agent::controlHead(cv::Mat raw, vector<Observation> observations)
{
  bool foundBall = false;
  Vector2f foundBallAtPx;
  for (Observation const& obs : observations)
  {
    if (obs.type == O_BALL)
    {
      foundBall = true;
      foundBallAtPx = obs.pos;
    }
  }
  if (foundBall)
  {
    float r = 0.9;
    Vector2f centerPx = Vector2f(raw.cols/2, raw.rows/2);
    Vector2f offset = (foundBallAtPx - centerPx) * r;
    offset.x() *= (Camera::VIEW_H_ANGLE / (double)raw.cols); // pixel per angle
    offset.y() *= (Camera::VIEW_V_ANGLE / (double)raw.rows); // pixel per angle
//    std::cout << "Found ball at " << foundBallAtPx.x() << "," << foundBallAtPx.y() << " - "
//              << "Corresponds to an offset of " << offset.x() << "," << offset.y() << std::endl;
    Head::GetInstance()->MoveTracking(Point2D(offset.x(), offset.y()));
  }
  d_debugger.setIsBallObserved(foundBall);
}
#include "localiser.ih"

void Localiser::updateSmoothedPos()
{
  double thetaX = cos(d_pos.theta());
  double thetaY = sin(d_pos.theta());
  auto smoothed = d_avgPos.next(Vector4d(d_pos.x(), d_pos.y(), thetaX, thetaY));

  double newTheta = atan2(smoothed[3], smoothed[2]);

  d_smoothedPos = AgentPosition(smoothed.x(), smoothed.y(), newTheta);
}
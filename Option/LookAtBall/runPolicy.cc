#include "lookatball.ih"

std::vector<std::shared_ptr<Option>> LookAtBall::runPolicy()
{
  auto const& ballObs = AgentState::get<CameraFrameState>()->getBallObservation();

  if (!ballObs.hasValue())
  {
//     log::warning("LookAtBall::runPolicy") << "No ball observation in AgentFrame yet LookAtBall was run";
    return std::vector<std::shared_ptr<Option>>();
  }

  Vector2d ballPos = ballObs->head<2>();

  static float r = 0.85;

  static unsigned w = d_cameraModel->imageWidth();
  static unsigned h = d_cameraModel->imageHeight();

  static Vector2d centerPx = Vector2d(w,h) / 2;
  static float happ = d_cameraModel->rangeHorizontalDegs() / w;
  static float vapp = d_cameraModel->rangeVerticalDegs() / h;

  Vector2d offset = (ballPos - centerPx) * r;

  offset.x() *= happ; // pixel per angle
  offset.y() *= vapp; // pixel per angle

  float maxOffset = 20;
  float minOffset = 2;

  offset = offset.cwiseMin(Vector2d(maxOffset,maxOffset)).cwiseMax(Vector2d(-maxOffset,-maxOffset));

//   cout << "offset: " << offset.transpose() << endl;
  
  if (offset.norm() < minOffset)
  {
    // The head is roughly looking at the ball so don't bother moving and
    // reset any accumulated state in the tracking logic (PID build up.)
    d_headModule->initTracking();
  }
  else
  {
    d_headModule->moveTracking(offset.x(), offset.y());
  }

  return std::vector<std::shared_ptr<Option>>();
}

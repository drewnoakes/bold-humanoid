#include "lookatball.ih"

std::vector<std::shared_ptr<Option>> LookAtBall::runPolicy()
{
  auto const& ballObs = AgentState::get<CameraFrameState>()->getBallObservation();

  if (!ballObs.hasValue())
  {
//     cerr << "[LookAtBall::runPolicy] No ball observation in AgentFrame yet LookAtBall was run" << endl;
    return std::vector<std::shared_ptr<Option>>();
  }

  Vector2d ballPos = ballObs->head<2>();

  static float r = 0.85;

  unsigned w = d_cameraModel->imageWidth();
  unsigned h = d_cameraModel->imageHeight();

  static Vector2d centerPx = Vector2d(w,h) / 2;
  static float happ = d_cameraModel->rangeHorizontalDegs() / w;
  static float vapp = d_cameraModel->rangeVerticalDegs() / h;

  Vector2d offset = (ballPos - centerPx) * r;

  offset.x() *= happ; // pixel per angle
  offset.y() *= vapp; // pixel per angle

  float maxOffset = 20;
  offset = offset.cwiseMin(Vector2d(maxOffset,maxOffset)).cwiseMax(Vector2d(-maxOffset,-maxOffset));

//   cout << "offset: " << offset.transpose() << endl;
  if (offset.norm() < 2)
    offset = Vector2d(0,0);

  d_headModule->moveTracking(offset.x(), offset.y());

  return std::vector<std::shared_ptr<Option>>();
}

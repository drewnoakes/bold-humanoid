#include "lookatgoal.ih"

std::vector<std::shared_ptr<Option>> LookAtGoal::runPolicy()
{
  auto const& goalObs = AgentState::get<CameraFrameState>()->getGoalObservations();

  if (goalObs.size() < 2)
  {
    cerr << ccolor::warning << "[LookAtGoal::runPolicy] Couldn't see both goal posts!" << ccolor::reset << endl;
    return std::vector<std::shared_ptr<Option>>();
  }

  auto middle = (goalObs[0] + goalObs[1]) / 2;

  static float r = 0.85;

  unsigned w = d_cameraModel->imageWidth();
  unsigned h = d_cameraModel->imageHeight();

  static Vector2d centerPx = Vector2d(w,h) / 2;
  static float happ = d_cameraModel->rangeHorizontalDegs() / w;
  static float vapp = d_cameraModel->rangeVerticalDegs() / h;

  Vector2d offset = (middle - centerPx) * r;

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

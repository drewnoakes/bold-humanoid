#include "lookatgoal.ih"

vector<shared_ptr<Option>> LookAtGoal::runPolicy(Writer<StringBuffer>& writer)
{
  auto const& goalObs = State::get<CameraFrameState>()->getGoalObservations();

  writer.String("goals").StartArray();
  for (auto const& goal : goalObs)
    writer.String("goals").StartArray().Double(goal.x()).Double(goal.y()).EndArray(2);
  writer.EndArray();

  if (goalObs.size() < 2)
  {
    log::warning("LookAtGoal::runPolicy") << "Couldn't see both goal posts!";
    return {};
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

  // TODO this max offset in config
  float maxOffset = 20;
  offset = offset.cwiseMin(Vector2d(maxOffset,maxOffset)).cwiseMax(Vector2d(-maxOffset,-maxOffset));

//   cout << "offset: " << offset.transpose() << endl;
  // TODO this min offset norm in config
  if (offset.norm() < 2)
    offset = Vector2d(0,0);

  d_headModule->moveTracking(offset.x(), offset.y());

  writer.String("offset").StartArray().Double(offset.x()).Double(offset.y()).EndArray(2);

  return {};
}

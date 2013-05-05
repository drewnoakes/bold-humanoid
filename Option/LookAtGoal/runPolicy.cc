#include "lookatgoal.ih"

OptionList LookAtGoal::runPolicy()
{
  auto const& goalObs = AgentState::get<CameraFrameState>()->getGoalObservations();

  if (goalObs.size() < 2)
  {
    cerr << "[LookAtBall::runPolicy] No ball seen!" << endl;
    return OptionList();
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

  Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
  Head::GetInstance()->MoveTracking(Point2D(offset.x(), offset.y()));

  return OptionList();
}

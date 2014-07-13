#include "lookatball.hh"

#include "../../CameraModel/cameramodel.hh"
#include "../../MotionModule/HeadModule/headmodule.hh"
#include "../../State/state.hh"
#include "../../StateObject/CameraFrameState/cameraframestate.hh"
#include "../../VisualCortex/visualcortex.hh"

using namespace bold;
using namespace Eigen;
using namespace std;
using namespace rapidjson;

LookAtBall::LookAtBall(std::string const& id, std::shared_ptr<CameraModel> cameraModel, std::shared_ptr<HeadModule> headModule)
: Option(id, "LookAtBall"),
  d_cameraModel(cameraModel),
  d_headModule(headModule),
  d_gain(Config::getSetting<double>("options.look-at-ball.gain")),
  d_minOffset(Config::getSetting<double>("options.look-at-ball.offset-min")),
  d_maxOffset(Config::getSetting<double>("options.look-at-ball.offset-max"))
{}

vector<shared_ptr<Option>> LookAtBall::runPolicy(Writer<StringBuffer>& writer)
{
  auto const& ballObs = State::get<CameraFrameState>()->getBallObservation();

  if (!ballObs.hasValue())
  {
    writer.String("ball").Null();
//     log::warning("LookAtBall::runPolicy") << "No ball observation in AgentFrame yet LookAtBall was run";
    return {};
  }

  static unsigned w = d_cameraModel->imageWidth();
  static unsigned h = d_cameraModel->imageHeight();

  static Vector2d centerPx = Vector2d(w,h) / 2;
  static double happ = d_cameraModel->rangeHorizontalDegs() / w;
  static double vapp = d_cameraModel->rangeVerticalDegs() / h;

  Vector2d ballPos = ballObs->head<2>();

  double r = d_gain->getValue();
  Vector2d offset = (ballPos - centerPx) * r;

  offset.x() *= happ; // pixel per angle
  offset.y() *= vapp; // pixel per angle

  double maxOffset = d_maxOffset->getValue();
  double minOffset = d_minOffset->getValue();

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

  writer.String("offset").StartArray().Double(offset.x()).Double(offset.y()).EndArray(2);

  return {};
}

void LookAtBall::reset()
{
  d_headModule->initTracking();
}

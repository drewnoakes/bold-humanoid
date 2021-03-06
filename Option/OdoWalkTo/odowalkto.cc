#include "odowalkto.hh"

#include "../../State/state.hh"
#include "../../StateObject/OdometryState/odometrystate.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

OdoWalkTo::OdoWalkTo(string const& id, shared_ptr<WalkModule> walkModule)
  : Option(id, "OdoWalkTo"),
  d_walkModule{move(walkModule)},
  d_targetPos{0.0, 0.0},
  d_targetFaceDir{0.0, 1.0},
  d_maxDist{0.1},
  d_lastOdoReading{Affine3d::Identity()},
  d_progress{Affine3d::Identity()}
{}

void OdoWalkTo::setTarget(Vector2d targetPos, Vector2d targetFaceDir, double maxDist)
{
  d_targetPos = targetPos;
  d_targetFaceDir = targetFaceDir;

  d_maxDist = maxDist;
  d_lastOdoReading = State::get<OdometryState>()->getTransform();
  d_progress = Affine3d::Identity();
}

double OdoWalkTo::hasTerminated()
{
  updateProgress();

  Vector3d targetPos3d;
  targetPos3d << d_targetPos, 0.0;
  Vector2d stillToGo = (d_progress * targetPos3d).head<2>();
  double dist = stillToGo.norm();

  // Progress translation is center of agent frame at t = 0 in current frame
  return dist < d_maxDist ? 1.0 : 0.0;
}

Option::OptionVector OdoWalkTo::runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer)
{
  writer.String("odo");
  writer.StartArray();
  writer.Double(d_lastOdoReading.translation().x(), "%.3f");
  writer.Double(d_lastOdoReading.translation().y(), "%.3f");
  writer.EndArray(2);

  writer.String("target");
  writer.StartArray();
  writer.Double(d_targetPos.x(), "%.3f");
  writer.Double(d_targetPos.y(), "%.3f");
  writer.EndArray(2);

  writer.String("progress");
  writer.StartArray();
  writer.Double(d_progress.translation().x(), "%.3f");
  writer.Double(d_progress.translation().y(), "%.3f");
  writer.Double(d_progress.matrix().diagonal().sum());
  writer.EndArray(3);

  // target pos in current agent frame
  Vector3d targetPos3d;
  targetPos3d << d_targetPos, 0.0;
  Vector2d stillToGo = (d_progress * targetPos3d).head<2>();
  writer.String("togo");
  writer.StartArray();
  writer.Double(stillToGo.x(), "%.3f");
  writer.Double(stillToGo.y(), "%.3f");
  writer.EndArray(2);

//  double dist = stillToGo.norm();

  Vector2d moveDir = 10 * stillToGo.normalized();
  writer.String("movedir");
  writer.StartArray();
  writer.Double(moveDir.x(), "%.3f");
  writer.Double(moveDir.y(), "%.3f");
  writer.EndArray(2);

  double turnAngle = -atan2(moveDir.x(), moveDir.y());

  d_walkModule->setMoveDir(moveDir.y(), 0);
  d_walkModule->setTurnAngle(18 * turnAngle);

  return {};
}

void OdoWalkTo::updateProgress()
{
  auto curOdoReading = State::get<OdometryState>()->getTransform();
  // $A_tA_{t-1}$
  auto delta = curOdoReading * d_lastOdoReading.inverse();
  // $A_tA_0 = A_tA_{t-1} * A_{t-1}A_0$
  d_progress = delta * d_progress;

  d_lastOdoReading = curOdoReading;
}


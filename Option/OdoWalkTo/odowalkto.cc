#include "odowalkto.hh"

#include "../../State/state.hh"
#include "../../StateObject/OdometryState/odometrystate.hh"

using namespace bold;
using namespace Eigen;
using namespace std;

OdoWalkTo::OdoWalkTo(string const& id, shared_ptr<WalkModule> walkModule)
  : Option(id, "OdoWalkTo"),
  d_walkModule{move(walkModule)},
  d_targetPos{0.0, 0.0, 0.0},
  d_maxDist{0.0},
  d_lastOdoReading{Affine3d::Identity()},
  d_progress{Affine3d::Identity()}
{}

void OdoWalkTo::setTargetPos(Vector3d targetPos, double maxDist)
{
  d_targetPos = targetPos;
  d_maxDist = maxDist;
  d_lastOdoReading = State::get<OdometryState>()->getTransform();
  d_progress = Affine3d::Identity();
}

double OdoWalkTo::hasTerminated()
{
  updateProgress();
  // Progress translation is center of agent frame at t = 0 in current frame
  auto moved = -d_progress.translation();
  auto dist = (d_targetPos - moved).head<2>().norm();
  return dist < d_maxDist ? 1.0 : 0.0;
}

Option::OptionVector OdoWalkTo::runPolicy()
{  
  // target pos in current agent frame
  auto stillToGo = (d_progress * d_targetPos).head<2>();
  auto dist = stillToGo.norm();

  d_walkModule->setMoveDir(stillToGo);
  d_walkModule->setTurnAngle(0);

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


#include "kick.hh"

#include "../MotionScript/motionscript.hh"

#include <Eigen/Core>

using namespace bold;
using namespace Eigen;
using namespace std;

std::vector<std::shared_ptr<Kick const>> Kick::d_allKicks;

void Kick::loadAll()
{
  // TODO load from config
  // TODO produce kick params experimentally using kick-learner role

  d_allKicks.push_back(make_shared<Kick const>(
    "forward-right",
    "./motionscripts/kick-right.json",
    Bounds2d(Vector2d(0, 0), Vector2d(0.1, 0.2)),
    Vector2d(0, 1.5)
  ));

  d_allKicks.push_back(make_shared<Kick const>(
    "forward-left",
    "./motionscripts/kick-left.json",
    Bounds2d(Vector2d(-0.1, 0), Vector2d(0, 0.2)),
    Vector2d(0, 1.5)
  ));

  d_allKicks.push_back(make_shared<Kick const>(
    "side-left",
    "./motionscripts/kick-side-left.json",
    Bounds2d(Vector2d(-0.1, 0), Vector2d(0.05, 0.2)),
    Vector2d(1.5, 1.5)
  ));

  d_allKicks.push_back(make_shared<Kick const>(
    "side-right",
    "./motionscripts/kick-side-right.json",
    Bounds2d(Vector2d(-0.05, 0), Vector2d(0.1, 0.2)),
    Vector2d(-1.5, 1.5)
  ));
}

Kick::Kick(string id, string scriptPath, Bounds2d ballBounds, Vector2d endPos)
: d_id(id),
  d_motionScript(MotionScript::fromFile(scriptPath)),
  d_ballBounds(ballBounds),
  d_endPos(endPos)
{}

Maybe<Vector2d> Kick::estimateEndPos(Vector2d const& ballPos) const
{
  // TODO end pos depends upon start pos -- need means of modelling this
  // TODO model probabilistically, not absolutely, to allow learning risk/reward

  if (!d_ballBounds.contains(ballPos))
    return Maybe<Vector2d>::empty();

  return Maybe<Vector2d>(d_endPos);
}

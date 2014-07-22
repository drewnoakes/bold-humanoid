#include "kick.hh"

#include "../MotionScript/motionscript.hh"
#include "../util/log.hh"

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
    Vector2d(0.0, 1.853),
    Vector2d(0.065, 0.106)
  ));

  d_allKicks.push_back(make_shared<Kick const>(
    "forward-left",
    "./motionscripts/kick-left.json",
    Bounds2d(Vector2d(-0.1, 0), Vector2d(0, 0.2)),
    Vector2d(0.0, 1.853),
    Vector2d(-0.065, 0.106)
  ));

  d_allKicks.push_back(make_shared<Kick const>(
    "cross-left",
    "./motionscripts/kick-cross-left.json",
    Bounds2d(Vector2d(-0.1, 0), Vector2d(0.02, 0.16)),
    Vector2d(1.002, 1.152),
    Vector2d(-0.006, 0.105)
  ));

  d_allKicks.push_back(make_shared<Kick const>(
    "cross-right",
    "./motionscripts/kick-cross-right.json",
    Bounds2d(Vector2d(-0.02, 0), Vector2d(0.1, 0.16)),
    Vector2d(-1.002, 1.152),
    Vector2d(0.006, 0.105)
  ));

  d_allKicks.push_back(make_shared<Kick const>(
    "cross-left-80",
    "./motionscripts/kick-cross-left-80.json",
    Bounds2d(Vector2d(-0.1, 0), Vector2d(0.02, 0.2)),
    Vector2d(1.0, 0.1),
    Vector2d(-0.006, 0.105)
  ));

  d_allKicks.push_back(make_shared<Kick const>(
    "cross-right-80",
    "./motionscripts/kick-cross-right-80.json",
    Bounds2d(Vector2d(-0.02, 0), Vector2d(0.1, 0.2)),
    Vector2d(-1.0, 0.1),
    Vector2d(0.006, 0.105)
  ));
}

shared_ptr<Kick const> Kick::getById(string id)
{
  auto it = std::find_if(
    d_allKicks.begin(),
    d_allKicks.end(),
    [id](shared_ptr<Kick const> const& kick) { return kick->getId() == id; });

  if (it == d_allKicks.end())
  {
    log::error("Kick::getById") << "Requested kick with unknown id: " << id;
    throw runtime_error("Requested kick with unknown id");
  }

  return *it;
}

Kick::Kick(string id, string scriptPath, Bounds2d ballBounds, Vector2d endPos, Vector2d idealBallPos)
: d_id(id),
  d_motionScript(MotionScript::fromFile(scriptPath)),
  d_ballBounds(ballBounds),
  d_endPos(endPos),
  d_idealBallPos(idealBallPos)
{}

Maybe<Vector2d> Kick::estimateEndPos(Vector2d const& ballPos) const
{
  // TODO end pos depends upon start pos -- need means of modelling this
  // TODO model probabilistically, not absolutely, to allow learning risk/reward

  if (!d_ballBounds.contains(ballPos))
    return Maybe<Vector2d>::empty();

  return Maybe<Vector2d>(d_endPos);
}

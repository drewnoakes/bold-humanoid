#include "squaredance.hh"

using namespace bold;
using namespace std;
using namespace Eigen;

SquareDance::SquareDance(string const& id, shared_ptr<WalkModule> walkModule)
  : Option(id, "SquareDance"),
    d_stage{FORWARD},
    d_odoWalkTo{make_shared<OdoWalkTo>(id + ".odowalkto", walkModule)}
{
}

Option::OptionVector SquareDance::runPolicy()
{
  if (d_odoWalkTo->hasTerminated() > 0)
  {
    d_stage = (Stage)(d_stage + 1);
    if (d_stage == RESTART)
      d_stage = FORWARD;
    
    
    d_odoWalkTo->setTarget(Vector2d(1.0, 0.0),Vector2d(1.0, 0.0), 0.2);

    /*
    switch (d_stage)
    {
    case FORWARD:
      break;
    case RIGHT:
      d_odoWalkTo->setTargetPos(Vector3d(1.0, 0.0, 0.0), 0.2);
      break;
    case BACKWARD:
      d_odoWalkTo->setTargetPos(Vector3d(0.0, -1.0, 0.0), 0.2);
      break;
    case LEFT:
      d_odoWalkTo->setTargetPos(Vector3d(-1.0, 0.0, 0.0), 0.2);
      break;
    }
    */
  }

  return {d_odoWalkTo};
}

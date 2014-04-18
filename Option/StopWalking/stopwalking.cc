#include "stopwalking.hh"

#include "../../MotionModule/WalkModule/walkmodule.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

vector<shared_ptr<Option>> StopWalking::runPolicy(Writer<StringBuffer>& writer)
{
  d_walkModule->setMoveDir(Eigen::Vector2d(0,0));
  d_walkModule->setTurnAngle(0);

  writer.String("walkRunning").Bool(d_walkModule->isRunning());

  return {};
}

double StopWalking::hasTerminated()
{
  return d_walkModule->isRunning() ? 0.0 : 1.0;
}

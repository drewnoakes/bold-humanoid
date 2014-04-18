#include "stopwalking.hh"

#include "../../MotionModule/WalkModule/walkmodule.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

vector<shared_ptr<Option>> StopWalking::runPolicy(Writer<StringBuffer>& writer)
{
  writer.String("immediately").Bool(d_stopImmediately);

  if (d_stopImmediately)
  {
    d_walkModule->stopImmediately();
  }
  else
  {
    d_walkModule->stop();

    writer.String("walkRunning").Bool(d_walkModule->isRunning());
  }

  return {};
}

double StopWalking::hasTerminated()
{
  return d_walkModule->isRunning() ? 0.0 : 1.0;
}

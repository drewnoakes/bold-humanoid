#include "actionoption.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

vector<shared_ptr<Option>> ActionOption::runPolicy(Writer<StringBuffer>& writer)
{
  writer.String("run");
  writer.Bool(d_needsRunning);

  if (d_needsRunning)
  {
    d_action();
    d_needsRunning = false;
  }

  return {};
}

void ActionOption::reset()
{
  d_needsRunning = true;
}

double ActionOption::hasTerminated()
{
  return !d_needsRunning;
}

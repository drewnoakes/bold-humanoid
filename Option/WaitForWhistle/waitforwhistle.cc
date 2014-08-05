#include "waitforwhistle.hh"

#include "../../Config/config.hh"

using namespace bold;
using namespace std;
using namespace rapidjson;

WaitForWhistle::WaitForWhistle(string id)
: Option(id, "WaitForWhistle")
{
  reset();

  d_windowType = Config::getSetting<WindowFunctionType>("whistle-detection.window-function");
  d_windowType->track([this] (WindowFunctionType type) { if (d_listener) d_listener->setWindowFunction(type); });
}

vector<shared_ptr<Option>> WaitForWhistle::runPolicy(Writer<StringBuffer>& writer)
{
  if (!d_initialised)
  {
    d_listener = make_unique<WhistleListener>();
    if (!d_listener->initialise())
    {
      // Failed to initialise sound device, so release and return (already logged)
      d_listener.reset();
      return {};
    }
    d_listener->setWindowFunction(d_windowType->getValue());
    d_initialised = true;
  }

  if (d_listener->step())
    d_terminated = true;

  return {};
}

double WaitForWhistle::hasTerminated()
{
  return d_terminated ? 1.0 : 0.0;
}

void WaitForWhistle::reset()
{
  d_initialised = false;
  d_terminated = false;

  if (d_listener)
    d_listener.reset();
}

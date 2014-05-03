#include "motionscriptoption.hh"

#include "../../MotionModule/MotionScriptModule/motionscriptmodule.hh"
#include "../../MotionScriptRunner/motionscriptrunner.hh"
#include "../../MotionScript/motionscript.hh"
#include "../../util/assert.hh"
#include "../../util/log.hh"

using namespace bold;
using namespace rapidjson;
using namespace std;

MotionScriptOption::MotionScriptOption(std::string const& id, std::shared_ptr<MotionScriptModule> const& motionScriptModule, std::string const& fileName, bool ifNotFinalPose)
: Option(id, "MotionScript"),
  d_motionScriptModule(motionScriptModule),
  d_hasTerminated(false),
  d_ifNotFinalPose(ifNotFinalPose)
{
  d_script = MotionScript::fromFile(fileName);

  if (!d_script)
  {
    log::error("MotionScriptOption::MotionScriptOption") << "Motion script file not found: " << fileName;
    throw runtime_error("File not found");
  }
}

double MotionScriptOption::hasTerminated()
{
  return d_hasTerminated ? 1.0 : 0.0;
}

vector<shared_ptr<Option>> MotionScriptOption::runPolicy(Writer<StringBuffer>& writer)
{
  writer.String("hasExisting").Bool(d_request != nullptr);

  if (!d_request)
  {
    // This is the first execution since being reset

    if (d_ifNotFinalPose && MotionScriptRunner::isInFinalPose(d_script))
    {
      // Don't run.
      writer.String("skip").String("Already in final pose");
      writer.String("maxDelta").Int(MotionScriptRunner::getMaxDeltaFromFinalPose(d_script));
      d_request = nullptr;
      d_hasTerminated = true;
      return {};
    }

    d_request = d_motionScriptModule->run(d_script);

    // TODO fix << operator for MotionRequestStatus

    log::verbose("MotionScriptOption::runPolicy") << "Motion script '" << getId() << "' requested with status: " << getMotionRequestStatusName(d_request->getStatus());

    writer.String("status").String(getMotionRequestStatusName(d_request->getStatus()).c_str());
  }
  else
  {
    // We're continuing a previous motion request. Check its status.

    writer.String("status").String(getMotionRequestStatusName(d_request->getStatus()).c_str());

    if (d_request->hasCompleted())
      d_hasTerminated = true;
  }

  return {};
}

void MotionScriptOption::reset()
{
  log::verbose("MotionScriptOption::reset") << getId();
  d_hasTerminated = false;
  d_request = nullptr;
}

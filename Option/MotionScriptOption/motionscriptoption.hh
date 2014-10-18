#pragma once

#include "../option.hh"

#include <string>

namespace bold
{
  class MotionRequest;
  class MotionScript;
  class MotionScriptModule;

  /// An option that requests a particular motion script be played when its
  /// policy is run.  Note that the motion scheduler may reject the request to
  /// play the script if the relevant body sections are already engaged.
  ///
  /// Indicates termination only when the script completes.
  class MotionScriptOption : public Option
  {
  public:
    MotionScriptOption(std::string const& id, std::shared_ptr<MotionScriptModule> const& motionScriptModule, std::string const& fileName = "", bool ifNotFinalPose = false);

    /// Returns a truthy value when the script has completed execution.
    double hasTerminated() override;

    std::vector<std::shared_ptr<Option>> runPolicy(rapidjson::Writer<rapidjson::StringBuffer>& writer) override;

    /// Clears any previous attempt at running a motion script. Subsequent
    /// calls to runPolicy will make a new request of the motion task scheduler.
    void reset() override;

    void setMotionScript(std::shared_ptr<MotionScript const> script) { d_script = script; }

  private:
    std::shared_ptr<MotionScriptModule> d_motionScriptModule;
    /// The script associated with this MotionScriptOption
    std::shared_ptr<MotionScript const> d_script;
    /// The most recent MotionRequest issued to the MotionScriptModule
    std::shared_ptr<MotionRequest const> d_request;
    bool d_hasTerminated;
    bool d_ifNotFinalPose;
  };
}

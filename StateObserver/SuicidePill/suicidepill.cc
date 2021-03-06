#include "suicidepill.hh"

#include "../../Agent/agent.hh"
#include "../../CM730Snapshot/cm730snapshot.hh"
#include "../../Debugger/debugger.hh"
#include "../../util/assert.hh"
#include "../../util/log.hh"

#include <stdlib.h>

using namespace bold;
using namespace std;

SuicidePill::SuicidePill(Agent* agent, std::shared_ptr<Debugger> debugger)
: TypedStateObserver<HardwareState>("Suicide Pill", ThreadId::MotionLoop),
  d_agent(agent),
  d_debugger(debugger),
  d_exited(false)
{
  ASSERT(agent);
  ASSERT(debugger);
}

void SuicidePill::observeTyped(shared_ptr<HardwareState const> const& state, SequentialTimer& timer)
{
  if (d_exited)
    return;

  const double stopProcessAfterSeconds = 3.0;
  const double stopServiceAfterSeconds = 6.0;

  auto const& cm730 = state->getCM730State();

  bool showDazzle = false;

  if (cm730.isStartButtonPressed && cm730.isModeButtonPressed)
  {
    // Both buttons pressed

    if (d_isActive)
    {
      double seconds = Clock::getSecondsSince(d_pressedAt);

      if (seconds > stopServiceAfterSeconds)
      {
        log::warning("SuicidePill::observeTyped") << "Both buttons held for " << stopServiceAfterSeconds << " seconds. Stopping service.";
        int res = system("stop boldhumanoid");
        if (res != 0)
          log::error("SuicidePill::observeTyped") << "System call to stop boldhumanoid service exited with: " << res;
        d_exited = true;
      }
      else if (seconds > stopProcessAfterSeconds)
      {
        showDazzle = true;
      }
    }
    else
    {
      log::info("SuicidePill::observeTyped") << "Both buttons pressed. Waiting...";
      d_isActive = true;
      d_pressedAt = Clock::getTimestamp();
    }
  }
  else
  {
    // Neither button pressed

    if (d_isActive)
    {
      d_isActive = false;

      double seconds = Clock::getSecondsSince(d_pressedAt);

      if (seconds > stopProcessAfterSeconds && seconds < stopServiceAfterSeconds)
      {
        log::warning("SuicidePill::observeTyped") << "Both buttons held for " << stopProcessAfterSeconds << " seconds. Stopping process.";
        d_agent->stop();
      }
    }
  }

  d_debugger->showDazzle(showDazzle);
}

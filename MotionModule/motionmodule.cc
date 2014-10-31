#include "motionmodule.hh"

#include "../MotionTaskScheduler/motiontaskscheduler.hh"

using namespace bold;
using namespace std;

MotionModule::MotionModule(std::string const& type, std::shared_ptr<MotionTaskScheduler> const& scheduler)
  : d_scheduler(scheduler),
    d_name(type),
    d_isCompleted(false)
{
  d_scheduler->registerModule(this);
}

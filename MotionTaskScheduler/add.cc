#include "motiontaskscheduler.ih"

void MotionTaskScheduler::add(MotionModule* module,
                              Priority headPriority, bool requestCommitHead,
                              Priority armsPriority, bool requestCommitArms,
                              Priority legsPriority, bool requestCommitLegs)
{
  assert(ThreadUtil::isThinkLoopThread());

  auto handleSection = [this,module](SectionId section, Priority priority, bool requestCommit)
  {
    if (priority == Priority::None)
      return;
    d_tasks.push_back(make_shared<MotionTask>(module, section, priority, requestCommit));
    d_hasChange = true;
  };

  handleSection(SectionId::Head, headPriority, requestCommitHead);
  handleSection(SectionId::Arms, armsPriority, requestCommitArms);
  handleSection(SectionId::Legs, legsPriority, requestCommitLegs);
}

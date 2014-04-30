#include "motiontaskstate.hh"

#include "../../MotionModule/motionmodule.hh"

#include <memory>

using namespace bold;
using namespace rapidjson;
using namespace std;

/*
  {
    head: [
      {
        module: "head",
        priority: 2,
        committed: false
      },
      {
        module: "walk",
        priority: 1,
        committed: true
      }
    ],
    arms: [
      {
        module: "walk",
        priority: 2,
        committed: true
      },
      {
        module: "motion-script",
        description: "Salute",
        priority: 1,
        committed: false
      }
    ],
    legs: [
      {
        module: "walk",
        priority: 2,
        committed: true
      },
      {
        module: "motion-script",
        description: "Salute",
        priority: 1,
        committed: false
      }
    ]
  }
*/

void MotionTaskState::writeJson(Writer<StringBuffer>& writer) const
{
  auto writeSection = [this,&writer](string name, SectionId section, shared_ptr<MotionTask> const& selected)
  {
    writer.String(name.c_str());
    writer.StartArray();
    {
      for (shared_ptr<MotionTask> const& task : d_allTasks)
      {
        if (task->getSection() != section)
          continue;

        writer.StartObject();
        {
          writer.String("module").String(task->getModule()->getName().c_str());
          writer.String("priority").Int((int)task->getPriority());
          writer.String("committed").Bool(task->isCommitted());
          writer.String("selected").Bool(task == selected);
        }
        writer.EndObject();
      }
    }
    writer.EndArray();
  };

  writer.StartObject();
  {
    writeSection("head", SectionId::Head, d_headTask);
    writeSection("arms", SectionId::Arms, d_armsTask);
    writeSection("legs", SectionId::Legs, d_legsTask);
  }
  writer.EndObject();
}

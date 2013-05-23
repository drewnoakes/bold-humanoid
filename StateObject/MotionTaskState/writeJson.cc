#include "motiontaskstate.hh"

#include "../MotionModule/motionmodule.hh"

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
        module: "action",
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
        module: "action",
        description: "Salute",
        priority: 1,
        committed: false
      }
    ]
  }
*/
  
void MotionTaskState::writeJson(Writer<StringBuffer>& writer) const
{
  auto writeSection = [&](string name, vector<shared_ptr<MotionTask>> const& tasks)
  {
    writer.String(name.c_str());
    writer.StartArray();
    {
      for (shared_ptr<MotionTask> const& task : tasks)
      {
        writer.StartObject();
        {
          writer.String("module").String(task->getModule()->getName().c_str());
          writer.String("priority").Int((int)task->getPriority());
          writer.String("committed").Bool(task->isCommitted());
        }
        writer.EndObject();
      }
    }
    writer.EndArray();
  };
  
  writer.StartObject();
  {
    writeSection("head", d_headTasks);
    writeSection("arms", d_armTasks);
    writeSection("legs", d_legTasks);
  }
  writer.EndObject();
}
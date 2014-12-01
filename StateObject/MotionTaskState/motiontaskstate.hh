#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "../stateobject.hh"
#include "../../MotionTask/motiontask.hh"
#include "../../MotionModule/motionmodule.hh"
#include "../../ThreadUtil/threadutil.hh"
#include "../../util/assert.hh"

namespace bold
{
  class MotionTaskState : public StateObject
  {
  public:
    MotionTaskState(
      std::shared_ptr<std::vector<std::pair<MotionModule*, std::shared_ptr<JointSelection>>>> moduleJointSelection,
      std::shared_ptr<MotionTask> headTask,
      std::shared_ptr<MotionTask> armsTask,
      std::shared_ptr<MotionTask> legsTask,
      std::vector<std::shared_ptr<MotionTask>> allTasks
    )
    : d_moduleJointSelection(moduleJointSelection),
      d_headTask(headTask),
      d_armsTask(armsTask),
      d_legsTask(legsTask),
      d_allTasks(allTasks)
    {
      ASSERT(ThreadUtil::isThinkLoopThread());
    }

    bool isEmpty() const { return d_moduleJointSelection->size() == 0; }

    /// Provides which modules to step, and what JointSelection to pass them
    std::shared_ptr<std::vector<std::pair<MotionModule*, std::shared_ptr<JointSelection>>>> getModuleJointSelection() const { return d_moduleJointSelection; }

    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::Writer<WebSocketBuffer>& writer) const override { writeJsonInternal(writer); }
    void writeJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer) const override { writeJsonInternal(writer); }

  private:
    template<typename TBuffer>
    void writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const;

    std::shared_ptr<std::vector<std::pair<MotionModule*, std::shared_ptr<JointSelection>>>> d_moduleJointSelection;
    std::shared_ptr<MotionTask> d_headTask;
    std::shared_ptr<MotionTask> d_armsTask;
    std::shared_ptr<MotionTask> d_legsTask;
    std::vector<std::shared_ptr<MotionTask>> d_allTasks;
  };

  template<typename TBuffer>
  inline void MotionTaskState::writeJsonInternal(rapidjson::Writer<TBuffer> &writer) const
  {
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

    auto writeSection = [this,&writer](std::string name, SectionId section, std::shared_ptr<MotionTask> const& selected)
    {
      writer.String(name.c_str());
      writer.StartArray();
      {
        for (std::shared_ptr<MotionTask> const& task : d_allTasks)
        {
          if (task->getSection() != section)
            continue;

          writer.StartObject();
          {
            writer.String("module");
            writer.String(task->getModule()->getName().c_str());
            writer.String("priority");
            writer.Int((int)task->getPriority());
            writer.String("committed");
            writer.Bool(task->isCommitted());
            writer.String("selected");
            writer.Bool(task == selected);
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
}

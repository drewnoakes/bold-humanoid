#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "../stateobject.hh"
#include "../../MotionTask/motiontask.hh"

namespace bold
{
  class MotionTaskState : public StateObject
  {
  public:
    MotionTaskState(
      std::shared_ptr<std::vector<std::pair<std::shared_ptr<MotionTask>, std::shared_ptr<JointSelection>>>> moduleJointSelection,
      std::vector<std::shared_ptr<MotionTask>> headTasks,
      std::vector<std::shared_ptr<MotionTask>> armTasks,
      std::vector<std::shared_ptr<MotionTask>> legTasks
    )
    : d_moduleJointSelection(moduleJointSelection),
      d_headTasks(headTasks),
      d_armTasks(armTasks),
      d_legTasks(legTasks)
    {}    
    
    /// Provides which modules to step, and what JointSelection to pass them
    std::shared_ptr<std::vector<std::pair<std::shared_ptr<MotionTask>, std::shared_ptr<JointSelection>>>> getModuleJointSelection() const { return d_moduleJointSelection; }
    
    void writeJson(rapidjson::Writer<rapidjson::StringBuffer>& writer) const override;

  private:
    std::shared_ptr<std::vector<std::pair<std::shared_ptr<MotionTask>, std::shared_ptr<JointSelection>>>> d_moduleJointSelection;
    std::vector<std::shared_ptr<MotionTask>> d_headTasks;
    std::vector<std::shared_ptr<MotionTask>> d_armTasks;
    std::vector<std::shared_ptr<MotionTask>> d_legTasks;
  };
}

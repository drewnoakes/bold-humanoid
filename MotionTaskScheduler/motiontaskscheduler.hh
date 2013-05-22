#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <vector>

#include "../AgentState/agentstate.hh"
#include "../MotionTask/motiontask.hh"
#include "../StateObject/MotionTaskState/motiontaskstate.hh"

namespace bold
{
  /** Tracks all active MotionTasks.
   * 
   * Thread safe.
   */
  class MotionTaskScheduler
  {
  public:
    MotionTaskScheduler()
    : d_hasChange(false)
    {}
    
    /** Adds a task to be picked up in the next motion loop.
     *
     * The highest priority task per body section will be selected.
     * If the task has a commit request and is selected, it will be set
     * as committed, until the corresponding MotionModule::step function
     * returns false.
     */
    void add(std::shared_ptr<MotionTask> task)
    {
      std::lock_guard<std::mutex> g(d_mutex);
      d_tasks.push_back(task);
      d_hasChange = true;
    }
    
    void remove(std::shared_ptr<MotionTask> task)
    {
      std::lock_guard<std::mutex> g(d_mutex);
      auto it = std::find(d_tasks.begin(), d_tasks.end(), task);
      
      if (it == d_tasks.end())
        return;
      
      d_tasks.erase(it);
      d_hasChange = true;
    }
    
    /** Called by the motion loop before reading MotionTaskState.
     * 
     * Updates MotionTaskState, if any changes have been made.
     */
    void update();

  private:
    std::mutex d_mutex;
    std::vector<std::shared_ptr<MotionTask>> d_tasks;
    bool d_hasChange;
  };  
}
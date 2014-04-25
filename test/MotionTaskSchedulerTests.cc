#include "gtest/gtest.h"

#include "../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../MotionModule/WalkModule/walkmodule.hh"

using namespace std;
using namespace bold;

TEST (MotionTaskSchedulerTests, sortTasks)
{
  auto scheduler = make_shared<MotionTaskScheduler>();
  WalkModule module(scheduler);

  auto task1 = make_shared<MotionTask>(&module, SectionId::Legs, Priority::Normal, true);
  auto task2 = make_shared<MotionTask>(&module, SectionId::Legs, Priority::Normal, true);
  auto task3 = make_shared<MotionTask>(&module, SectionId::Legs, Priority::Normal, true);

  vector<shared_ptr<MotionTask>> tasks = {task1, task2, task3};

  MotionTaskScheduler::sortTasks(tasks);

  EXPECT_EQ ( task1.get(), tasks[0].get() );
  EXPECT_EQ ( task2.get(), tasks[1].get() );
  EXPECT_EQ ( task3.get(), tasks[2].get() );

  task2->setCommitted();

  MotionTaskScheduler::sortTasks(tasks);

  EXPECT_EQ ( task2.get(), tasks[0].get() );
  EXPECT_EQ ( task1.get(), tasks[1].get() );
  EXPECT_EQ ( task3.get(), tasks[2].get() );

  task2->clearCommitted();
  task3->setCommitted();

  MotionTaskScheduler::sortTasks(tasks);

  EXPECT_EQ ( task3.get(), tasks[0].get() );
  EXPECT_EQ ( task2.get(), tasks[1].get() );
  EXPECT_EQ ( task1.get(), tasks[2].get() );

  task3->clearCommitted();
  task1->setCommitted();
  task2->setPriority(Priority::High);

  MotionTaskScheduler::sortTasks(tasks);

  EXPECT_EQ ( task1.get(), tasks[0].get() );
  EXPECT_EQ ( task2.get(), tasks[1].get() );
  EXPECT_EQ ( task3.get(), tasks[2].get() );
}

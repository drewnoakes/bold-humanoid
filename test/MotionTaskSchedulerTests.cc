#include "gtest/gtest.h"

#include "../MotionTaskScheduler/motiontaskscheduler.hh"
#include "../MotionModule/MotionScriptModule/motionscriptmodule.hh"
#include "../MotionModule/WalkModule/walkmodule.hh"
#include "../State/state.hh"

using namespace std;
using namespace bold;

class MotionTaskSchedulerTests : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    ThreadUtil::setThreadId(ThreadId::ThinkLoop);

    scheduler = make_shared<MotionTaskScheduler>();

    walkModule = new WalkModule(scheduler);
    motionScriptModule = new MotionScriptModule(scheduler);
  }

  virtual void TearDown()
  {
    delete walkModule;
    delete motionScriptModule;
  }

  shared_ptr<MotionTaskScheduler> scheduler;
  WalkModule* walkModule;
  MotionScriptModule* motionScriptModule;
};

TEST_F (MotionTaskSchedulerTests, sortTasks)
{
  auto task1 = make_shared<MotionTask>(walkModule, SectionId::Legs, Priority::Normal, true);
  auto task2 = make_shared<MotionTask>(walkModule, SectionId::Legs, Priority::Normal, true);
  auto task3 = make_shared<MotionTask>(walkModule, SectionId::Legs, Priority::Normal, true);

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

TEST_F (MotionTaskSchedulerTests, addAndUpdate)
{
  scheduler->add(walkModule,
                 Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
                 Priority::Normal, Required::Yes, RequestCommit::No,   // ARMS
                 Priority::Normal, Required::Yes, RequestCommit::Yes); // LEGS

  scheduler->update();

  EXPECT_EQ ( walkModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getLegTask()->getModule() );

  EXPECT_FALSE ( scheduler->getHeadTask()->isCommitRequested() );
  EXPECT_FALSE ( scheduler->getArmTask()->isCommitRequested() );
  EXPECT_TRUE  ( scheduler->getLegTask()->isCommitRequested() );

  EXPECT_FALSE ( scheduler->getHeadTask()->isCommitted() );
  EXPECT_FALSE ( scheduler->getArmTask()->isCommitted() );
  EXPECT_TRUE  ( scheduler->getLegTask()->isCommitted() );

  // Next think cycle...

  scheduler->add(motionScriptModule,
                 Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
                 Priority::Normal, Required::Yes, RequestCommit::Yes,  // ARMS
                 Priority::Normal, Required::Yes, RequestCommit::Yes); // LEGS

  scheduler->update();

  // Because the leg section is busy with a committed task, the entire new
  // request is ignored.
  EXPECT_EQ ( nullptr, scheduler->getHeadTask() );
  EXPECT_EQ ( nullptr, scheduler->getArmTask() );
  EXPECT_EQ ( walkModule, scheduler->getLegTask()->getModule() );
}

TEST_F (MotionTaskSchedulerTests, twoInOneCycle_perfectTie)
{
  ASSERT_EQ ( nullptr, scheduler->getHeadTask() );
  ASSERT_EQ ( nullptr, scheduler->getArmTask() );
  ASSERT_EQ ( nullptr, scheduler->getLegTask() );

  scheduler->add(walkModule,
                 Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
                 Priority::Normal, Required::Yes, RequestCommit::Yes,  // ARMS
                 Priority::Normal, Required::Yes, RequestCommit::Yes); // LEGS

  scheduler->add(motionScriptModule,
                 Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
                 Priority::Normal, Required::Yes, RequestCommit::Yes,  // ARMS
                 Priority::Normal, Required::Yes, RequestCommit::Yes); // LEGS

  scheduler->update();

  // As both are identical from a priority perspective, take the first

  EXPECT_EQ ( walkModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getLegTask()->getModule() );
}

TEST_F (MotionTaskSchedulerTests, twoInOneCycle_tieOtherThanRequired)
{
  scheduler->add(walkModule,
                 Priority::Normal, Required::No, RequestCommit::No,   // HEAD
                 Priority::Normal, Required::No, RequestCommit::Yes,  // ARMS
                 Priority::Normal, Required::No, RequestCommit::Yes); // LEGS

  scheduler->add(motionScriptModule,
                 Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
                 Priority::Normal, Required::Yes, RequestCommit::Yes,  // ARMS
                 Priority::Normal, Required::Yes, RequestCommit::Yes); // LEGS

  scheduler->update();

  // The first doesn't specify any requirement, but it is still selected as
  // require level doesn't effect ordering.

  EXPECT_EQ ( walkModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getLegTask()->getModule() );
}

TEST_F (MotionTaskSchedulerTests, twoInOneCycle_winSomeLoseSomeButRequireLowerPriority)
{
  scheduler->add(walkModule,
                 Priority::High, Required::No,  RequestCommit::No,  // HEAD
                 Priority::High, Required::No,  RequestCommit::No,  // ARMS
                 Priority::Low,  Required::Yes, RequestCommit::No); // LEGS

  scheduler->add(motionScriptModule,
                 Priority::Normal, Required::No, RequestCommit::No,  // HEAD
                 Priority::Normal, Required::No, RequestCommit::No,  // ARMS
                 Priority::Normal, Required::No, RequestCommit::No); // LEGS

  scheduler->update();

  EXPECT_EQ ( walkModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getLegTask()->getModule() );

  scheduler->add(walkModule,
                 Priority::High, Required::No,  RequestCommit::No,  // HEAD
                 Priority::Low,  Required::Yes, RequestCommit::No,  // ARMS
                 Priority::High, Required::No,  RequestCommit::No); // LEGS

  scheduler->add(motionScriptModule,
                 Priority::Normal, Required::No, RequestCommit::No,  // HEAD
                 Priority::Normal, Required::No, RequestCommit::No,  // ARMS
                 Priority::Normal, Required::No, RequestCommit::No); // LEGS

  scheduler->update();

  EXPECT_EQ ( walkModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getLegTask()->getModule() );

  scheduler->add(walkModule,
                 Priority::Low,  Required::Yes, RequestCommit::No,  // HEAD
                 Priority::High, Required::No,  RequestCommit::No,  // ARMS
                 Priority::High, Required::No,  RequestCommit::No); // LEGS

  scheduler->add(motionScriptModule,
                 Priority::Normal, Required::No, RequestCommit::No,  // HEAD
                 Priority::Normal, Required::No, RequestCommit::No,  // ARMS
                 Priority::Normal, Required::No, RequestCommit::No); // LEGS

  scheduler->update();

  EXPECT_EQ ( walkModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getLegTask()->getModule() );
}

TEST_F (MotionTaskSchedulerTests, twoInOneCycle_priorityDecides)
{
  scheduler->add(walkModule,
                 Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
                 Priority::Normal, Required::Yes, RequestCommit::No,   // ARMS
                 Priority::Normal, Required::Yes, RequestCommit::Yes); // LEGS

  scheduler->add(motionScriptModule,
                 Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
                 Priority::Normal, Required::Yes, RequestCommit::Yes,  // ARMS
                 Priority::High,   Required::Yes, RequestCommit::Yes); // LEGS

  scheduler->update();

  // The motion script module has high priority for the legs

  EXPECT_EQ ( motionScriptModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( motionScriptModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( motionScriptModule, scheduler->getLegTask()->getModule() );
}

TEST_F (MotionTaskSchedulerTests, twoInOneCycle_conflict)
{
  scheduler->add(walkModule,
                 Priority::Normal, Required::No,  RequestCommit::No,  // HEAD
                 Priority::High,   Required::Yes, RequestCommit::No,  // ARMS
                 Priority::Normal, Required::Yes, RequestCommit::No); // LEGS

  scheduler->add(motionScriptModule,
                 Priority::Normal, Required::No,  RequestCommit::No,  // HEAD
                 Priority::Normal, Required::Yes, RequestCommit::No,  // ARMS
                 Priority::High,   Required::Yes, RequestCommit::No); // LEGS

  scheduler->update();

  // The motion script module has high priority for the legs

  EXPECT_EQ ( walkModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getLegTask()->getModule() );
}

TEST_F (MotionTaskSchedulerTests, addRequiredAtopCommittedOtherBodySection)
{
  scheduler->add(walkModule,
                 Priority::Normal, Required::No, RequestCommit::Yes, // HEAD
                 Priority::Normal, Required::No, RequestCommit::No,  // ARMS
                 Priority::Normal, Required::No, RequestCommit::No); // LEGS

  scheduler->update();

  scheduler->add(motionScriptModule,
                 Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
                 Priority::Normal, Required::Yes, RequestCommit::Yes,  // ARMS
                 Priority::Normal, Required::Yes, RequestCommit::Yes); // LEGS

  scheduler->update();

  EXPECT_EQ ( walkModule,         scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( motionScriptModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( motionScriptModule, scheduler->getLegTask()->getModule() );

  EXPECT_TRUE ( scheduler->getHeadTask()->isCommitted() );
  EXPECT_TRUE ( scheduler->getArmTask()->isCommitted() );
  EXPECT_TRUE ( scheduler->getLegTask()->isCommitted() );
}

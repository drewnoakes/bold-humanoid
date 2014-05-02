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
  auto request = make_shared<MotionRequest>();
  auto task1 = make_shared<MotionTask>(request, walkModule, SectionId::Legs, Priority::Normal, true);
  auto task2 = make_shared<MotionTask>(request, walkModule, SectionId::Legs, Priority::Normal, true);
  auto task3 = make_shared<MotionTask>(request, walkModule, SectionId::Legs, Priority::Normal, true);

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
  auto request1 = scheduler->request(
    walkModule,
    Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
    Priority::Normal, Required::Yes, RequestCommit::No,   // ARMS
    Priority::Normal, Required::Yes, RequestCommit::Yes); // LEGS

  EXPECT_EQ ( MotionRequestStatus::Pending, request1->getStatus() );

  scheduler->update();

  EXPECT_EQ ( MotionRequestStatus::Selected, request1->getStatus() );

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

  auto request2 = scheduler->request(
    motionScriptModule,
    Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
    Priority::Normal, Required::Yes, RequestCommit::Yes,  // ARMS
    Priority::Normal, Required::Yes, RequestCommit::Yes); // LEGS

  EXPECT_EQ ( MotionRequestStatus::Pending, request2->getStatus() );

  scheduler->update();

  EXPECT_EQ ( MotionRequestStatus::Ignored, request2->getStatus() );

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

  auto request1 = scheduler->request(
    walkModule,
    Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
    Priority::Normal, Required::Yes, RequestCommit::Yes,  // ARMS
    Priority::Normal, Required::Yes, RequestCommit::Yes); // LEGS

  auto request2 = scheduler->request(
    motionScriptModule,
    Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
    Priority::Normal, Required::Yes, RequestCommit::Yes,  // ARMS
    Priority::Normal, Required::Yes, RequestCommit::Yes); // LEGS

  scheduler->update();

  // As both are identical from a priority perspective, take the first

  EXPECT_EQ ( MotionRequestStatus::Selected, request1->getStatus() );
  EXPECT_EQ ( MotionRequestStatus::Ignored, request2->getStatus() );

  EXPECT_EQ ( walkModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getLegTask()->getModule() );
}

TEST_F (MotionTaskSchedulerTests, twoInOneCycle_tieOtherThanRequired)
{
  auto request1 = scheduler->request(
    walkModule,
    Priority::Normal, Required::No, RequestCommit::No,   // HEAD
    Priority::Normal, Required::No, RequestCommit::Yes,  // ARMS
    Priority::Normal, Required::No, RequestCommit::Yes); // LEGS

  auto request2 = scheduler->request(
    motionScriptModule,
    Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
    Priority::Normal, Required::Yes, RequestCommit::Yes,  // ARMS
    Priority::Normal, Required::Yes, RequestCommit::Yes); // LEGS

  scheduler->update();

  // The first doesn't specify any requirement, but it is still selected as
  // require level doesn't effect ordering.

  EXPECT_EQ ( MotionRequestStatus::Selected, request1->getStatus() );
  EXPECT_EQ ( MotionRequestStatus::Ignored, request2->getStatus() );

  EXPECT_EQ ( walkModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getLegTask()->getModule() );
}

TEST_F (MotionTaskSchedulerTests, twoInOneCycle_winSomeLoseSomeButRequireLowerPriority)
{
  auto request1 = scheduler->request(
    walkModule,
    Priority::High, Required::No,  RequestCommit::No,  // HEAD
    Priority::High, Required::No,  RequestCommit::No,  // ARMS
    Priority::Low,  Required::Yes, RequestCommit::No); // LEGS

  auto request2 = scheduler->request(
    motionScriptModule,
    Priority::Normal, Required::No, RequestCommit::No,  // HEAD
    Priority::Normal, Required::No, RequestCommit::No,  // ARMS
    Priority::Normal, Required::No, RequestCommit::No); // LEGS

  scheduler->update();

  EXPECT_EQ ( MotionRequestStatus::Completed, request1->getStatus() ); // completed already as no committment requested
  EXPECT_EQ ( MotionRequestStatus::Ignored, request2->getStatus() );

  EXPECT_EQ ( walkModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getLegTask()->getModule() );

  auto request3 = scheduler->request(
    walkModule,
    Priority::High, Required::No,  RequestCommit::No,  // HEAD
    Priority::Low,  Required::Yes, RequestCommit::No,  // ARMS
    Priority::High, Required::No,  RequestCommit::No); // LEGS

  auto request4 = scheduler->request(
    motionScriptModule,
    Priority::Normal, Required::No, RequestCommit::No,  // HEAD
    Priority::Normal, Required::No, RequestCommit::No,  // ARMS
    Priority::Normal, Required::No, RequestCommit::No); // LEGS

  scheduler->update();

  EXPECT_EQ ( MotionRequestStatus::Completed, request3->getStatus() ); // completed already as no committment requested
  EXPECT_EQ ( MotionRequestStatus::Ignored, request4->getStatus() );

  EXPECT_EQ ( walkModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getLegTask()->getModule() );

  auto request5 = scheduler->request(
    walkModule,
    Priority::Low,  Required::Yes, RequestCommit::No,  // HEAD
    Priority::High, Required::No,  RequestCommit::No,  // ARMS
    Priority::High, Required::No,  RequestCommit::No); // LEGS

  auto request6 = scheduler->request(
    motionScriptModule,
    Priority::Normal, Required::No, RequestCommit::No,  // HEAD
    Priority::Normal, Required::No, RequestCommit::No,  // ARMS
    Priority::Normal, Required::No, RequestCommit::No); // LEGS

  scheduler->update();

  EXPECT_EQ ( MotionRequestStatus::Completed, request5->getStatus() ); // completed already as no committment requested
  EXPECT_EQ ( MotionRequestStatus::Ignored, request6->getStatus() );

  EXPECT_EQ ( walkModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getLegTask()->getModule() );
}

TEST_F (MotionTaskSchedulerTests, twoInOneCycle_priorityDecides)
{
  auto request1 = scheduler->request(
    walkModule,
    Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
    Priority::Normal, Required::Yes, RequestCommit::No,   // ARMS
    Priority::Normal, Required::Yes, RequestCommit::Yes); // LEGS

  auto request2 = scheduler->request(
    motionScriptModule,
    Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
    Priority::Normal, Required::Yes, RequestCommit::Yes,  // ARMS
    Priority::High,   Required::Yes, RequestCommit::Yes); // LEGS

  scheduler->update();

  // The motion script module has high priority for the legs

  EXPECT_EQ ( MotionRequestStatus::Ignored, request1->getStatus() );
  EXPECT_EQ ( MotionRequestStatus::Selected, request2->getStatus() );

  EXPECT_EQ ( motionScriptModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( motionScriptModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( motionScriptModule, scheduler->getLegTask()->getModule() );
}

TEST_F (MotionTaskSchedulerTests, twoInOneCycle_conflict)
{
  auto request1 = scheduler->request(
    walkModule,
    Priority::Normal, Required::No,  RequestCommit::No,  // HEAD
    Priority::High,   Required::Yes, RequestCommit::No,  // ARMS
    Priority::Normal, Required::Yes, RequestCommit::No); // LEGS

  auto request2 = scheduler->request(
    motionScriptModule,
    Priority::Normal, Required::No,  RequestCommit::No,  // HEAD
    Priority::Normal, Required::Yes, RequestCommit::No,  // ARMS
    Priority::High,   Required::Yes, RequestCommit::No); // LEGS

  scheduler->update();

  // The motion script module has high priority for the legs

  EXPECT_EQ ( MotionRequestStatus::Completed, request1->getStatus() ); // completed already as no committment requested
  EXPECT_EQ ( MotionRequestStatus::Ignored, request2->getStatus() );

  EXPECT_EQ ( walkModule, scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( walkModule, scheduler->getLegTask()->getModule() );
}

TEST_F (MotionTaskSchedulerTests, addRequiredAtopCommittedOtherBodySection)
{
  auto request1 = scheduler->request(
    walkModule,
    Priority::Normal, Required::No, RequestCommit::Yes, // HEAD
    Priority::Normal, Required::No, RequestCommit::No,  // ARMS
    Priority::Normal, Required::No, RequestCommit::No); // LEGS

  scheduler->update();

  EXPECT_EQ ( MotionRequestStatus::Selected, request1->getStatus() );

  auto request2 = scheduler->request(
    motionScriptModule,
    Priority::Normal, Required::No,  RequestCommit::No,   // HEAD
    Priority::Normal, Required::Yes, RequestCommit::Yes,  // ARMS
    Priority::Normal, Required::Yes, RequestCommit::Yes); // LEGS

  scheduler->update();

  EXPECT_EQ ( MotionRequestStatus::Selected, request1->getStatus() );
  EXPECT_EQ ( MotionRequestStatus::Selected, request2->getStatus() );

  EXPECT_EQ ( walkModule,         scheduler->getHeadTask()->getModule() );
  EXPECT_EQ ( motionScriptModule, scheduler->getArmTask()->getModule() );
  EXPECT_EQ ( motionScriptModule, scheduler->getLegTask()->getModule() );

  EXPECT_TRUE ( scheduler->getHeadTask()->isCommitted() );
  EXPECT_TRUE ( scheduler->getArmTask()->isCommitted() );
  EXPECT_TRUE ( scheduler->getLegTask()->isCommitted() );
}

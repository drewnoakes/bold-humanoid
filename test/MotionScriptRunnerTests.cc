#include <gtest/gtest.h>

#include "../CM730Snapshot/cm730snapshot.hh"
#include "../MotionScript/motionscript.hh"
#include "../MotionScriptRunner/motionscriptrunner.hh"
#include "../MotionTask/motiontask.hh"
#include "../MX28Snapshot/mx28snapshot.hh"
#include "../State/state.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../ThreadUtil/threadutil.hh"

#include <memory>
#include <vector>

#include <Eigen/Core>

using namespace bold;
using namespace Eigen;
using namespace std;

void pushStep(shared_ptr<MotionScript::Stage> stage, ushort value, uchar moveCycles, uchar pauseCycles = 0)
{
  MotionScript::KeyFrame step;

  step.moveCycles = moveCycles;
  step.pauseCycles = pauseCycles;

  for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
    step.values[jointId-1] = value;

  stage->keyFrames.push_back(step);
}

// TODO unit test that runs through all motion script files on disk

TEST (DISABLED_MotionScriptRunnerTests, basics)
{
  ThreadUtil::setThreadId(ThreadId::MotionLoop);

  // TODO convenience method for populating a basic HardwareState object
  auto cm730State = make_unique<CM730Snapshot const>();
  auto mx28States = vector<unique_ptr<MX28Snapshot const>>();
  for (uchar id = 0; id < 20; id++) {
    auto mx28 = make_unique<MX28Snapshot>(id);
    mx28->presentPositionValue = 0;
    mx28States.push_back(move(mx28));
  }
  State::set<HardwareState>(make_shared<HardwareState>(move(cm730State), move(mx28States), 0, 0, 0));

  auto stage = make_shared<MotionScript::Stage>();

  pushStep(stage, 100, 5, 0);

  vector<shared_ptr<MotionScript::Stage>> stages = { stage };
  auto script = make_shared<MotionScript>("test-script", stages, true, true, true);

  MotionScriptRunner runner(script);

  EXPECT_EQ(MotionScriptRunnerStatus::Pending, runner.getStatus());
  EXPECT_EQ(0, runner.getCurrentStageIndex());
  EXPECT_EQ(0, runner.getCurrentKeyFrameIndex());
  EXPECT_EQ(script, runner.getScript());

  EXPECT_TRUE(runner.step(JointSelection::all()));

  EXPECT_EQ(MotionScriptRunnerStatus::Running, runner.getStatus());

  while (runner.step(JointSelection::all()))
  {}
}

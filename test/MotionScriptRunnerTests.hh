#include "gtest/gtest.h"

#include "../AgentState/agentstate.hh"
#include "../CM730Snapshot/cm730snapshot.hh"
#include "../MotionScript/motionscript.hh"
#include "../MotionScriptRunner/motionscriptrunner.hh"
#include "../MotionTask/motiontask.hh"
//#include "../MX28/mx28.hh"
#include "../MX28Snapshot/mx28snapshot.hh"
#include "../StateObject/HardwareState/hardwarestate.hh"
#include "../ThreadId/threadid.hh"

#include "helpers.hh"

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

TEST (MotionScriptRunnerTests, double)
{
  ThreadId::setThreadId(ThreadId::ThreadIds::MotionLoop);

  // TODO convenience method for populating a basic HardwareState object
  AgentState::getInstance().registerStateType<HardwareState>("Hardware");
  auto cm730State = make_shared<CM730Snapshot>();
  auto mx28States = vector<shared_ptr<MX28Snapshot const>>();
  for (int i = 0; i < 20; i++) {
    auto mx28 = make_shared<MX28Snapshot>();
    mx28->presentPositionValue = 0;
    mx28States.push_back(mx28);
  }
  AgentState::getInstance().set<HardwareState>(make_shared<HardwareState>(cm730State, mx28States, 0, 0));

  auto stage = make_shared<MotionScript::Stage>();

  pushStep(stage, 100, 5, 0);

  vector<shared_ptr<MotionScript::Stage>> stages = { stage };
  auto script = make_shared<MotionScript>("test-script", stages);

  MotionScriptRunner runner(script);

  EXPECT_EQ(MotionScriptRunnerState::Pending, runner.getState());
  EXPECT_EQ(0, runner.getCurrentStageIndex());
  EXPECT_EQ(0, runner.getCurrentStepIndex());
  EXPECT_EQ("test-script", runner.getScriptName());

  EXPECT_TRUE(runner.step(JointSelection::all()));

  EXPECT_EQ(MotionScriptRunnerState::Running, runner.getState());

}

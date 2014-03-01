#include "agent.ih"

void Agent::registerStateTypes()
{
  State::registerStateType<AgentFrameState>("AgentFrame");
  State::registerStateType<AmbulatorState>("Ambulator");
  State::registerStateType<BodyControlState>("BodyControl");
  State::registerStateType<BodyState>("Body");
  State::registerStateType<CameraFrameState>("CameraFrame");
  State::registerStateType<DebugState>("Debug");
  State::registerStateType<GameState>("Game");
  State::registerStateType<HardwareState>("Hardware");
  State::registerStateType<LabelCountState>("LabelCount");
  State::registerStateType<MotionTaskState>("MotionTask");
  State::registerStateType<MotionTimingState>("MotionTiming");
  State::registerStateType<OdometryState>("Odometry");
  State::registerStateType<OptionTreeState>("OptionTree");
  State::registerStateType<OrientationState>("Orientation");
  State::registerStateType<ParticleState>("Particle");
  State::registerStateType<StaticHardwareState>("StaticHardware");
  State::registerStateType<ThinkTimingState>("ThinkTiming");
  State::registerStateType<WorldFrameState>("WorldFrame");
}

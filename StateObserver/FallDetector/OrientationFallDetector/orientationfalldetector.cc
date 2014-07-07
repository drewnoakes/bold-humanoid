#include "orientationfalldetector.hh"

#include "../../../Config/config.hh"
#include "../../../ThreadUtil/threadutil.hh"

using namespace bold;
using namespace std;

OrientationFallDetector::OrientationFallDetector(shared_ptr<Voice> voice)
: FallDetector(voice),
  TypedStateObserver<OrientationState>("Fall detector", ThreadId::MotionLoop)
{}

void OrientationFallDetector::observeTyped(std::shared_ptr<OrientationState const> const& orientationState, SequentialTimer& timer)
{
  double pitch = orientationState->getPitchAngle();
  double roll = orientationState->getRollAngle();

  static auto pitchThreshold = Config::getSetting<double>("fall-detector.orientation.pitch-threshold");
  static auto rollThreshold = Config::getSetting<double>("fall-detector.orientation.roll-threshold");

  double pitchBreach = pitchThreshold->getValue() - fabs(pitch);
  double rollBreach = rollThreshold->getValue() - fabs(roll);

  if (pitchBreach > 0 && rollBreach > 0)
  {
    setFallState(FallState::STANDUP);
  }
  else if (pitchBreach < rollBreach)
  {
    setFallState(pitch < 0 ? FallState::FORWARD : FallState::BACKWARD);
  }
  else
  {
    setFallState(roll > 0 ? FallState::LEFT : FallState::RIGHT);
  }
}

void OrientationFallDetector::logFallData(stringstream& msg) const
{
  auto const& orientationState = State::get<OrientationState>();

  msg << round(Math::radToDeg(orientationState->getPitchAngle())) << ","
      << round(Math::radToDeg(orientationState->getRollAngle())) << ","
      << round(Math::radToDeg(orientationState->getYawAngle()));
}

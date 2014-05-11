#include "falldetector.hh"

#include "../../CM730Snapshot/cm730snapshot.hh"
#include "../../Config/config.hh"
#include "../../Voice/voice.hh"

#include <iomanip>

using namespace bold;
using namespace std;

FallDetector::FallDetector(shared_ptr<Voice> voice)
: TypedStateObserver<HardwareState>("Fall detector", ThreadId::MotionLoop),
  d_voice(voice),
  d_fbAvgValue(Config::getStaticValue<int>("fall-detector.window-size")),
  d_lrAvgValue(Config::getStaticValue<int>("fall-detector.window-size")),
  d_maxLimitValue(Config::getSetting<int>("fall-detector.max-limit-value")),
  d_turnFbRatio(Config::getSetting<double>("fall-detector.turn-fb-ratio")),
  d_fallenState(FallState::STANDUP),
  d_startTime(Clock::getTimestamp())
{}

void FallDetector::observeTyped(std::shared_ptr<HardwareState const> const& hardwareState, SequentialTimer& timer)
{
  // Track the smoothed forward/backward acceleration to test for a consistent
  // indication that we have fallen.

  int fbAvg =   d_fbAvgValue.next(hardwareState->getCM730State().accRaw.y()) - d_neutralAccPos;
  int lrAvg = -(d_lrAvgValue.next(hardwareState->getCM730State().accRaw.x()) - d_neutralAccPos);

  if (d_fbAvgValue.isMature())
  {
    bool standingBefore = d_fallenState == FallState::STANDUP;

    int dist = Eigen::Vector2d(fbAvg, lrAvg).norm();

    // Update our estimate of whether we've fallen or not
    if (dist > d_maxLimitValue->getValue())
    {
      // Only do the turn movement if we are really on our side
      double scaledFbAvg = fbAvg * d_turnFbRatio->getValue();
      d_fallenState = abs(scaledFbAvg) > abs(lrAvg)
        ? (scaledFbAvg > 0 ? FallState::FORWARD : FallState::BACKWARD)
        : (lrAvg       > 0 ? FallState::LEFT    : FallState::RIGHT);
    }
    else
    {
      d_fallenState = FallState::STANDUP;
    }

    if (standingBefore && d_fallenState != FallState::STANDUP && d_voice->queueLength() == 0)
    {
      // Log a bunch of data when a fall is detected
      stringstream msg;
      msg << setprecision(3) << Clock::getSecondsSince(d_startTime) << ","
          << getFallStateName(d_fallenState) << "," << fbAvg << "," << lrAvg
          << "," << hardwareState->getCM730State().voltage;

      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
        msg << "," << (int)hardwareState->getMX28State(jointId).presentTemp;

      log::info("fall-data") << msg.str();

      d_voice->sayOneOf({"Ouch!", "Dammit", "Ooopsy", "Bah", "Why", "Not again"});
    }
  }
}

string bold::getFallStateName(FallState fallState)
{
  switch (fallState)
  {
    case FallState::STANDUP:
      return "Standup";
    case FallState::BACKWARD:
      return "Backward";
    case FallState::FORWARD:
      return "Forward";
    case FallState::LEFT:
      return "Left";
    case FallState::RIGHT:
      return "Right";
    default:
      return "Unknown";
  }
}

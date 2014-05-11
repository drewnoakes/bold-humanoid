#include "falldetector.hh"

#include "../../CM730/cm730.hh"
#include "../../CM730Snapshot/cm730snapshot.hh"
#include "../../Config/config.hh"
#include "../../Voice/voice.hh"

#include <iomanip>

using namespace bold;
using namespace std;

FallDetector::FallDetector(shared_ptr<Voice> voice)
: TypedStateObserver<HardwareState>("Fall detector", ThreadId::MotionLoop),
  d_voice(voice),
  d_xAvg(Config::getStaticValue<int>("fall-detector.window-size")),
  d_yAvg(Config::getStaticValue<int>("fall-detector.window-size")),
  d_zAvg(Config::getStaticValue<int>("fall-detector.window-size")),
  d_fallenState(FallState::STANDUP),
  d_startTime(Clock::getTimestamp())
{}

void FallDetector::observeTyped(std::shared_ptr<HardwareState const> const& hardwareState, SequentialTimer& timer)
{
  // Track the smoothed acceleration along each axis to test for a consistent
  // indication that we have fallen.

  auto const& accRaw = hardwareState->getCM730State().accRaw;

  int xAvg = d_xAvg.next(accRaw.x()) - CM730::ACC_VALUE_MID;
  int yAvg = d_yAvg.next(accRaw.y()) - CM730::ACC_VALUE_MID;
  int zAvg = d_zAvg.next(accRaw.z()) - CM730::ACC_VALUE_MID;

  if (d_xAvg.isMature())
  {
    ASSERT(d_yAvg.isMature() && d_zAvg.isMature());

    bool standingBefore = d_fallenState == FallState::STANDUP;

    // TODO add some hysterisis here to avoid flickering between states (seen as multiple consecutive fall-data log entries)

    if (abs(zAvg) > abs(xAvg) && abs(zAvg) > abs(yAvg))
    {
      // NOTE could actually be standing on our head, but this is quite unlikely :)
      d_fallenState = FallState::STANDUP;
    }
    else if (abs(xAvg) > abs(yAvg))
    {
      d_fallenState = xAvg > 0 ? FallState::RIGHT : FallState::LEFT;
    }
    else
    {
      d_fallenState = yAvg > 0 ? FallState::FORWARD : FallState::BACKWARD;
    }

    if (standingBefore && d_fallenState != FallState::STANDUP)
    {
      // Log a bunch of data when a fall is detected
      stringstream msg;
      msg << setprecision(3) << Clock::getSecondsSince(d_startTime) << ","
          << getFallStateName(d_fallenState) << ","
          << xAvg << "," << yAvg << "," << zAvg << ","
          << hardwareState->getCM730State().voltage;

      for (uchar jointId = (uchar)JointId::MIN; jointId <= (uchar)JointId::MAX; jointId++)
        msg << "," << (int)hardwareState->getMX28State(jointId).presentTemp;

      log::info("fall-data") << msg.str();

      // Announce the fall
      if (d_voice->queueLength() == 0)
        d_voice->sayOneOf({"Ouch", "Dammit", "Ooopsy", "Bah", "Why me", "Not again", "That hurt"});
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
